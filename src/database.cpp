/************************************************************************************
 * 
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023 Craig Edwards <brain@ssod.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/

#include <ssod/database.h>
#include <ssod/config.h>
#include <mysql/mysql.h>
#include <fmt/format.h>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <array>
#include <atomic>
#include <thread>
#include <condition_variable>

#ifdef MARIADB_VERSION_ID
	#define CONNECT_STRING "SET @@SESSION.max_statement_time=3000"
#else
	#define CONNECT_STRING "SET @@SESSION.max_execution_time=3000"
#endif

namespace db {

	using namespace std::literals::chrono_literals;

	/**
	 * @brief Represents a cached prepared statement.
	 * We need to store the MYSQL_STMT*, the bound variable pointers,
	 * and the lengths, plus buffers for char* bound variables.
	 */
	struct cached_query {
		/**
		 * @brief If true, this query expects results, e.g. SELECT, EXPLAIN
		 */
		bool expects_results{false};

		/**
		 * @brief The mysql statement handle
		 */
		MYSQL_STMT* st{nullptr};

		/**
		 * @brief C pointers to bound variables
		 */
		MYSQL_BIND* bindings{nullptr};

		/**
		 * @brief C pointers to bound variable lengths
		 */
		unsigned long* lengths{nullptr};

		/**
		 * @brief Used to represent a set of C string buffers
		 */
		std::vector<std::string> bufs;
	};

	/**
	* @brief Number of independent MySQL connections in the pool.
	* Each connection maintains its own prepared statement cache.
	*/
	constexpr size_t POOL_SIZE = 5;

	struct sqlconn {
		/**
		 * @brief MySQL database connection
		 */
		MYSQL connection{};

		/**
		 * @brief Database mutex
		 * one connection may only be accessed by one thread at a time!
		 */
		std::mutex mutex;

		/**
		 * @brief Query cache, a map of cached_query
		 */
		std::map<std::string, cached_query> cached_queries;

		/**
		 * @brief Whether this connection is currently established
		 */
		bool connected{false};
	};

	/**
	 * @brief Pool of independent database connections
	 */
	std::array<sqlconn, POOL_SIZE> connections;

	/**
	 * @brief Atomic round-robin starting index for connection selection
	 */
	std::atomic<size_t> curr_index{0};

	/**
	 * @brief Total number of queries since connection
	 */
	std::atomic<size_t> query_total{0};

	/**
	 * @brief Creating D++ cluster, used for logging
	 */
	dpp::cluster* creator{nullptr};

	using sql_query_callback = std::function<void(const resultset)>;

	std::condition_variable sql_worker_cv;
	std::mutex result_cache_mutex;

	/**
	 * @brief Cached query result parameters
	 */
	struct cached_query_results {
		std::string format;
		paramlist parameters;
		sql_query_callback callback;
	};

	/**
	 * @brief Cached query result records with expiry
	 */
	struct cached_query_result_set {
		const resultset results;
		const double expiry;
	};

#ifdef DPP_CORO
	std::queue<cached_query_results> sql_query_queue;
	std::mutex query_queue_mtx;
#endif

	template<class> inline constexpr bool always_false_v = false;

	struct cached_query_hash {
		std::size_t operator()(const cached_query_results& k) const {
			size_t x = std::hash<std::string>()(k.format);
			for (const auto& param : k.parameters) {
				std::visit([&x](auto &&p) {
					using T = std::decay_t<decltype(p)>;
					// float, std::string, uint64_t, int64_t, bool, int32_t, uint32_t, double
					if constexpr (std::is_same_v<T, float>) {
						x ^= std::hash<float>()(p);
					} else if constexpr (std::is_same_v<T, std::string>) {
						x ^= std::hash<std::string>()(p);
					} else if constexpr (std::is_same_v<T, uint64_t>) {
						x ^= std::hash<uint64_t>()(p);
					} else if constexpr (std::is_same_v<T, int64_t>) {
						x ^= std::hash<int64_t>()(p);
					} else if constexpr (std::is_same_v<T, bool>) {
						x ^= std::hash<bool>()(p);
					} else if constexpr (std::is_same_v<T, int32_t>) {
						x ^= std::hash<int32_t>()(p);
					} else if constexpr (std::is_same_v<T, uint32_t>) {
						x ^= std::hash<uint32_t>()(p);
					} else if constexpr (std::is_same_v<T, double>) {
						x ^= std::hash<double>()(p);
					} else {
						static_assert(always_false_v<T>, "non-exhaustive visitor!");
					}
				}, param);
			}
			return x;
		}
	};
	
	struct cached_query_equal {
		bool operator()(const cached_query_results& lhs, const cached_query_results& rhs) const {
			if (lhs.format != rhs.format || lhs.parameters.size() != rhs.parameters.size()) {
				return false;
			}
			return lhs.parameters == rhs.parameters;
		}
	};

	/**
	 * @brief A map of cached, executed queries with results and expiries
	 */
	std::unordered_map<cached_query_results, cached_query_result_set, cached_query_hash, cached_query_equal> cached_query_res;

	size_t cache_size() {
		size_t total = 0;
		for (auto& conn : connections) {
			std::lock_guard<std::mutex> lock(conn.mutex);
			total += conn.cached_queries.size();
		}
		return total;
	}

	size_t query_count() {
		return query_total.load();
	}

	/**
	 * @brief Release all prepared statements and their associated
	 * binding allocations for a single database connection.
	 *
	 * Called when closing a connection or before reconnecting,
	 * since prepared statement handles cannot survive reconnection.
	 *
	 * The caller must hold the connection mutex.
	 */
	void clear_statements(sqlconn& conn) {
		for (const auto& cc : conn.cached_queries) {
			mysql_stmt_close(cc.second.st);
			delete[] cc.second.bindings;
			delete[] cc.second.lengths;
		}
		conn.cached_queries.clear();
	}

	/**
	 * @brief This is an internal connect function which has no locking, there is no public interface for this
	 * 
	 * @param host hostname
	 * @param user username
	 * @param pass password
	 * @param db database
	 * @param port port
	 * @param socket unix socket
	 */
	bool unsafe_connect(sqlconn& conn, const std::string& host, const std::string& user, const std::string& pass, const std::string& db, int port, const std::string& socket) {
		if (mysql_init(&conn.connection) == nullptr) {
			creator->log(dpp::ll_error, "SQL: mysql_init() failed");
			return false;
		}
		mysql_options(&conn.connection, MYSQL_INIT_COMMAND, CONNECT_STRING);
		int opts = CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS | CLIENT_REMEMBER_OPTIONS | CLIENT_IGNORE_SIGPIPE;
		conn.connected = mysql_real_connect(&conn.connection, host.c_str(), user.c_str(), pass.c_str(), db.c_str(), port, socket.empty() ? nullptr : socket.c_str(), opts) != nullptr;
		if (!conn.connected) {
			creator->log(dpp::ll_error, mysql_error(&conn.connection));
		}
		return conn.connected;
	}

	bool connect(const std::string& host, const std::string& user, const std::string& pass, const std::string& db, int port, const std::string& socket) {
		for (auto& conn : connections) {
			std::lock_guard<std::mutex> lock(conn.mutex);
			if (!unsafe_connect(conn, host, user, pass, db, port, socket)) {
				return false;
			}
		}
		signal(SIGPIPE, SIG_IGN);
		return true;
	}

	struct selected_connection {
		sqlconn& conn;
		std::unique_lock<std::mutex> lock;
	};

	/**
	 * @brief Select a database connection from the pool.
	 *
	 * Start at the next round-robin connection and attempt to acquire
	 * an available connection without blocking. If all connections
	 * are busy, wait for the originally selected connection.
	 *
	 * The returned object owns the connection lock, ensuring that
	 * the connection and its prepared statements remain exclusively
	 * accessible until the object goes out of scope.
	 */
	selected_connection select_connection() {
		const size_t start = curr_index.fetch_add(1, std::memory_order_relaxed) % POOL_SIZE;
		for (size_t i = 0; i < POOL_SIZE; ++i) {
			auto& conn = connections[(start + i) % POOL_SIZE];
			std::unique_lock<std::mutex> lock(conn.mutex, std::try_to_lock);
			if (lock.owns_lock()) {
				return {conn, std::move(lock)};
			}
		}
		auto& conn = connections[start];
		return {conn, std::unique_lock<std::mutex>(conn.mutex)};
	}

#ifdef DPP_CORO
	void query_callback(const std::string &format, const paramlist &parameters, const sql_query_callback& cb) {
		{
			std::unique_lock<std::mutex> queue_lock(query_queue_mtx);
			sql_query_queue.emplace(std::move(cached_query_results{.format = format, .parameters = parameters, .callback = cb}));
		}
		sql_worker_cv.notify_one();
	}

	dpp::async<resultset> co_query(const std::string &format, const paramlist &parameters) {
		return dpp::async<resultset>{ [format, parameters] <typename C> (C &&cc) { return query_callback(format, parameters, std::forward<C>(cc)); }};
	}
#endif

	void init (dpp::cluster& bot) {
		creator = &bot;
		const json dbconf = config::get("database");
		if (!db::connect(dbconf["host"], dbconf["username"], dbconf["password"], dbconf["database"], dbconf["port"], dbconf.contains("socket") ? dbconf["socket"] : "")) {
			creator->log(dpp::ll_critical, fmt::format("Database connection error connecting to {}: {}", dbconf["database"].get<std::string>(), "See preceding SQL error"));
			exit(2);
		}
#ifdef DPP_CORO
		/**
		 * Start one SQL worker per pooled connection.
		 * Workers consume queries from the shared queue and execute them
		 * using whichever database connection is available.
		 *
		 * Query callbacks run after execution, allowing suspended
		 * coroutines to resume with their completed resultsets.
		 */
		for (size_t worker = 0; worker < POOL_SIZE; ++worker) {
			std::thread([&bot, worker]() {
				dpp::utility::set_thread_name("sql/coro/" + std::to_string(worker));
				while (true) {
					cached_query_results qr;
					{
						std::unique_lock<std::mutex> queue_lock(query_queue_mtx);
						sql_worker_cv.wait(queue_lock, [] {
							return !sql_query_queue.empty();
						});
						if (sql_query_queue.empty()) {
							continue;
						}
						qr = std::move(sql_query_queue.front());
						sql_query_queue.pop();
					}
					if (!qr.format.empty()) {
						auto results = query(qr.format, qr.parameters);
						if (qr.callback) {
							try {
								qr.callback(results);
							}
							catch (const std::exception& e) {
								bot.log(dpp::ll_warning, "Exception in SQL query callback: " + std::string(e.what()));
							}
						}
					}
				}
			}).detach();
		}
#endif
		creator->log(dpp::ll_info, fmt::format("Connected to database: {}", dbconf["database"].get<std::string>()));
	}

	/**
	 * @brief Certain types of query are not supported in the binary protocol
	 * used for prepared statements. As such they must be executed 'raw', and
	 * unprepared. This is not exposed to the interface as we don't want users
	 * directly calling raw_query() and summoning Little Bobby Tables. Only
	 * queries which return no result set are supported.
	 * 
	 * @return true query executed
	 */
	bool raw_query(const std::string& query) {
		auto selected = select_connection();
		return mysql_real_query(&selected.conn.connection, query.c_str(), query.length()) == 0;
	}

	bool start_transaction() {
		return raw_query("START TRANSACTION");
	}

	bool commit() {
		return raw_query("COMMIT");
	}

	bool rollback() {
		return raw_query("ROLLBACK");
	}

	void transaction(std::function<bool()> closure) {

	}

	bool close() {
		for (auto& conn : connections) {
			std::lock_guard<std::mutex> lock(conn.mutex);
			clear_statements(conn);
			if (conn.connected) {
				mysql_close(&conn.connection);
				conn.connected = false;
			}
		}
		mysql_library_end();
		return true;
	}

	void log_error(const std::string& format, const std::string& error) {
		const std::string message = format.empty() ? error : fmt::format("{} (query: {})", error, format);
		creator->log(dpp::ll_error, message);
	}

	resultset query(const std::string &format, const paramlist &parameters, double lifetime) {
		double now = dpp::utility::time_f();
		cached_query_results r{ .format = format, .parameters = parameters, .callback = nullptr };
		{
			std::lock_guard<std::mutex> lock(result_cache_mutex);
			auto f = cached_query_res.find(r);
			if (f != cached_query_res.end()) {
				if (now < f->second.expiry) {
					return f->second.results;
				}
				cached_query_res.erase(f);
			}
		}
		resultset rs = query(format, parameters);
		{
			std::lock_guard<std::mutex> lock(result_cache_mutex);
			cached_query_res.erase(r);
			cached_query_res.emplace(std::move(r), cached_query_result_set{ .results = rs, .expiry = now + lifetime });
		}
		return rs;
	}

	resultset query(const std::string &format, const paramlist &parameters) {

		/**
		 * Select and lock a database connection for the entire query.
		 * Prepared statements and their bindings belong to that connection,
		 * so they must not be accessed concurrently.
		 *
		 * The lock is released automatically when the query returns.
		 */
		auto selected = select_connection();
		sqlconn& conn = selected.conn;
		MYSQL& connection = conn.connection;
		auto& cached_queries = conn.cached_queries;
		resultset rv;

		/**
		 * Reconnect only the selected connection if it has failed.
		 * Its cached prepared statements must be discarded because
		 * their handles belong to the previous MySQL session.
		 */
		if (mysql_ping(&connection)) {
			creator->log(dpp::ll_error, "SQL: Connection has died, reconnecting...");
			clear_statements(conn);
			mysql_close(&connection);
			conn.connected = false;
			const json dbconf = config::get("database");
			if (!unsafe_connect(conn, dbconf["host"], dbconf["username"], dbconf["password"], dbconf["database"], dbconf["port"], dbconf.contains("socket") ? dbconf["socket"] : "")) {
				creator->log(dpp::ll_critical, fmt::format("Database connection error connecting to {}: {}", dbconf["database"].get<std::string>(), mysql_error(&connection)));
				rv.error = mysql_error(&connection);
				return rv;
			}
		}

		++query_total;

		double start = dpp::utility::time_f();

		/**
		 * Check for a cached query in the query cache, if one is found, we can use it,
		 * and we don't need to call mysql_stmt_init() and mysql_stmt_prepare().
		 */
		cached_query cc;
		auto f = cached_queries.find(format);
		if (f != cached_queries.end()) {
			/* Query already exists in prepared statement cache */
			cc = f->second;
		} else {

			/* Query doesn't exist yet, initialise a prepared statement and allocate char buffers */
			cc.st = mysql_stmt_init(&connection);
			if (mysql_stmt_prepare(cc.st, format.c_str(), format.length())) {
				log_error(format, mysql_stmt_error(cc.st));
				rv.error = mysql_stmt_error(cc.st);
				mysql_stmt_close(cc.st);
				return rv;
			}

			/* Check the parameter count provided matches that which MySQL expects */
			size_t expected_param_count = mysql_stmt_param_count(cc.st);
			if (parameters.size() != expected_param_count) {
				rv.error = "Incorrect number of parameters: " + format + " (" + std::to_string(parameters.size()) + " vs " + std::to_string(expected_param_count) + ")";
				log_error(format, rv.error);
				mysql_stmt_close(cc.st);
				return rv;
			}

			/* Allocate memory for awful C stuff 🐉 */
			cc.bindings = nullptr;
			cc.lengths = nullptr;
			if (expected_param_count) {
				cc.bindings = new MYSQL_BIND[expected_param_count];
				cc.lengths = new unsigned long[expected_param_count];
				memset(cc.lengths, 0, sizeof(unsigned long) * expected_param_count);
			}

			/* Determine if this query expects results by the first keyword */
			std::vector<std::string> q = (dpp::utility::tokenize(dpp::trim(dpp::lowercase(format)), " "));
			cc.expects_results = (q.size() > 0 && (q[0] == "select" || q[0] == "show" || q[0] == "describe" || q[0] == "explain"));

			/* Store to cache */
			cached_queries.emplace(format, cc);
			creator->log(dpp::ll_debug, "SQL: New cached prepared statement: " + format);
		}

		if (parameters.size()) {

			/* Parameters are expected for this query, bind them to the prepared statement */
			memset(cc.bindings, 0, sizeof(*cc.bindings));
			cc.bufs.clear();
			cc.bufs.reserve(parameters.size());
			int v = 0;
			for (const auto& param : parameters) {
				std::visit([&cc, &v](auto &&p) {
					using T = std::decay_t<decltype(p)>;
					if constexpr (std::is_same_v<T, std::string>) {
						cc.bufs.emplace_back(p);
					} else {
						cc.bufs.emplace_back(std::to_string(p));
					}
					cc.lengths[v] = cc.bufs[v].length();
					cc.bindings[v].buffer_type = MYSQL_TYPE_VAR_STRING;
					cc.bindings[v].buffer = const_cast<char*>(cc.bufs[v].c_str());
					cc.bindings[v].buffer_length = cc.bufs[v].length() + 1;
					cc.bindings[v].is_null = nullptr;
					cc.bindings[v].length = &cc.lengths[v];
					v++;
				}, param);
			}

			/* Bind parameters to statement */
			if (mysql_stmt_bind_param(cc.st, (MYSQL_BIND*)cc.bindings)) {
				log_error(format, mysql_stmt_error(cc.st));
				rv.error = mysql_stmt_error(cc.st);
				return rv;
			}
		}

		int result{0};
		//void *qlog = sentry::get_user_transaction();
		//void* qspan = sentry::db_span(qlog, format, cc.bufs);

		if (!cc.expects_results) {
			/**
			 * Query which does not expect results, e.g. UPDATE, INSERT
			 */
			result = mysql_stmt_execute(cc.st);
			if (result) {
				log_error(format, mysql_stmt_error(cc.st));
				rv.error = mysql_stmt_error(cc.st);
			} else {
				rv.affected_rows = mysql_stmt_affected_rows(cc.st);
				rv.insert_id = mysql_stmt_insert_id(cc.st);
			}
		} else {
			/**
			 * Query which expects results, e.g. SELECT
			 */
			unsigned long field_count{0};
			MYSQL_RES *a_res = mysql_stmt_result_metadata(cc.st);
			if (a_res) {
				field_count = mysql_stmt_field_count(cc.st);
				MYSQL_FIELD *fields = mysql_fetch_fields(a_res);
				MYSQL_BIND bindings[field_count];
				char* string_buffers[field_count];
				unsigned long lengths[field_count];
				bool is_null[field_count];
				std::memset(bindings, 0, sizeof(bindings));
				std::memset(is_null, 0, sizeof(is_null));
				std::memset(lengths, 0, sizeof(lengths));
				for (unsigned long i = 0; i < field_count; ++i) {
					/* FIX: Cap max length of retrieved field at 128k, else it will try and new[]
					 * a 4gb buffer, which is really REALLY slow!
					 */
					fields[i].length = std::min(fields[i].length, 131072UL);
					string_buffers[i] = new char[fields[i].length];
					memset(string_buffers[i], 0, fields[i].length);
					bindings[i].buffer_type = MYSQL_TYPE_VAR_STRING;
					bindings[i].buffer = string_buffers[i];
					bindings[i].buffer_length = fields[i].length;
					bindings[i].is_null = &is_null[i];
					bindings[i].length = &lengths[i];
				}

				result = mysql_stmt_bind_result(cc.st, (MYSQL_BIND*)&bindings);
				if (result) {
					rv.error = mysql_stmt_error(cc.st);
					log_error(format, rv.error);
					for (unsigned long i = 0; i < field_count; ++i) {
						delete[] string_buffers[i];
					}
					mysql_free_result(a_res);
					//sentry::end_span(qspan);
					return rv;
				}

				result = mysql_stmt_execute(cc.st);
				if (result == 0) {

					/* Build resultset */
					while (true) {
						result = mysql_stmt_fetch(cc.st); 
						if (result == MYSQL_NO_DATA) {
							/* End of resultset */
							break; 
						} else if (result != 0) {
							/* Error retrieving resultset, e.g. disconnected */
							rv.error = mysql_stmt_error(cc.st);
							log_error(format, rv.error);
							break; 
						}

						/* Build row*/
						if (mysql_num_fields(a_res)) {
							long s_field_count = 0;
							db::row thisrow;
							while (s_field_count < mysql_num_fields(a_res)) {
								std::string a = (fields[s_field_count].name ? fields[s_field_count].name : "");
								std::string b = is_null[s_field_count] ? "" : std::string(string_buffers[s_field_count], lengths[s_field_count]);
								thisrow.emplace(a, b);
								s_field_count++;
							}
							if (!thisrow.empty()) {
								rv.emplace_back(thisrow);
							} else {
								rv.error = "DB: Spurious empty fieldset";
								log_error(format, rv.error);
							}
						}
					}
				} else {
					rv.error = mysql_stmt_error(cc.st);
					log_error(format, rv.error);
				}
				mysql_free_result(a_res);
				for (unsigned long i = 0; i < field_count; ++i) {
					delete[] string_buffers[i];
				}
				//sentry::set_span_status(qspan, sentry::STATUS_OK);
			} else {
				log_error(format, mysql_stmt_error(cc.st));
				rv.error = mysql_stmt_error(cc.st);
				//sentry::set_span_status(qspan, sentry::STATUS_INVALID_ARGUMENT);
			}
		}

		if (dpp::utility::time_f() - start > 0.75) {
			creator->log(dpp::ll_warning, "Query took > 0.75 seconds: " + format);
		}

		//sentry::end_span(qspan);

		return rv;
	}
};
