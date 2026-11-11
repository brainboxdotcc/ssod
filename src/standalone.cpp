#include <ssod/standalone.h>
#include <ssod/database.h>
#include <dpp/dispatcher.h>
#include <openssl/sha.h>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <openssl/rand.h>
#include <shared_mutex>
#include <unordered_set>
#include <stdexcept>

namespace ssod {
namespace {
	using json = nlohmann::json;
	using connection_ptr = drogon::WebSocketConnectionPtr;

	struct client {
		uint64_t user_id{};
		bool authenticating{};
	};

	std::mutex connections_mutex;
	std::unordered_map<uint64_t, connection_ptr> connections;
	dpp::cluster *cluster{};
	std::atomic<uint64_t> serial{0};

	std::shared_mutex linked_users_mutex;
	std::unordered_set<uint64_t> linked_users;

	uint64_t next_id() {
		const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		return (1ULL << 63) | ((static_cast<uint64_t>(now) & ((1ULL << 41) - 1)) << 22) | (serial.fetch_add(1, std::memory_order_relaxed) & ((1ULL << 22) - 1));
	}

	std::string token_hash(const std::string &token) {
		unsigned char digest[SHA256_DIGEST_LENGTH];
		SHA256(reinterpret_cast<const unsigned char *>(token.data()), token.size(), digest);

		static constexpr char hex[] = "0123456789abcdef";
		std::string result;
		result.reserve(SHA256_DIGEST_LENGTH * 2);

		for (unsigned char c: digest) {
			result += hex[c >> 4];
			result += hex[c & 15];
		}

		return result;
	}

	bool valid_token(const std::string &token) {
		if (token.size() != 64) {
			return false;
		}

		for (char c: token) {
			if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
				return false;
			}
		}

		return true;
	}

	void send_error(const connection_ptr &conn, const std::string &reason) {
		if (conn->connected()) {
			conn->send(json{{"error", reason}}.dump());
		}
	}

void finish_authentication(const connection_ptr &conn, uint64_t id) {
		if (id == 0) {
			send_error(conn, "Invalid standalone user ID");
			return;
		}

		{
			std::lock_guard lock(connections_mutex);
			auto state = conn->getContext<client>();

			if (!state || !state->authenticating || !conn->connected()) {
				return;
			}

			state->user_id = id;
			state->authenticating = false;
			connections[id] = conn;
		}

		if (conn->connected()) {
			conn->send(json{
			    {"type", "authenticated"},
			    {"user_id", std::to_string(id)}
			}.dump());
		}

		standalone::input(id, {
		    {"type", 2},
		    {
			"data", {
			    {"type", 1},
			    {"name", "start"}
			}
		    }
		});
	}

void authenticate(const connection_ptr &conn, const std::string &token) {
    const std::string hash = token_hash(token);

    /* First check whether this is a linked Discord account */
    db::query_callback(
        "SELECT user_id FROM standalone_discord_auth WHERE token_hash = ?",
        {hash},
        [conn, hash](const db::resultset linked) {
            if (!linked.error.empty()) {
                send_error(conn, "Authentication unavailable");
                return;
            }

            if (!linked.rows.empty()) {
                try {
                    finish_authentication(
                        conn,
                        std::stoull(linked[0].at("user_id"))
                    );
                } catch (const std::exception &) {
                    send_error(conn, "Invalid linked user ID");
                }
                return;
            }

            /* Existing standalone registration and recovery path */
            db::query_callback(
                "INSERT IGNORE INTO standalone_auth (token_hash) VALUES (?)",
                {hash},
                [conn, hash](const db::resultset inserted) {
                    if (!inserted.error.empty()) {
                        send_error(conn, "Registration unavailable");
                        return;
                    }

                    db::query_callback(
                        "SELECT user_id FROM standalone_auth WHERE token_hash = ?",
                        {hash},
                        [conn](const db::resultset result) {
                            if (!result.error.empty() || result.rows.empty()) {
                                send_error(conn, "Authentication unavailable");
                                return;
                            }

                            uint64_t id;

                            try {
                                id = std::stoull(result[0].at("user_id"));
                            } catch (const std::exception &) {
                                send_error(conn, "Invalid standalone user ID");
                                return;
                            }

                            finish_authentication(conn, id);
                        }
                    );
                }
            );
        }
    );
}

}

bool standalone::is_linked(dpp::snowflake user_id) {
	std::shared_lock lock(linked_users_mutex);
	return linked_users.contains(static_cast<uint64_t>(user_id));
}

bool standalone::is_connected(dpp::snowflake user_id) {
	std::lock_guard lock(connections_mutex);
	auto it = connections.find(static_cast<uint64_t>(user_id));
	return it != connections.end() && it->second->connected();
}

dpp::task<std::string> standalone::link(dpp::snowflake user_id) {
	const auto existing = co_await db::co_query(
	    "SELECT user_id FROM standalone_discord_auth WHERE user_id = ?",
	    {user_id}
	);

	if (!existing.error.empty()) {
		throw std::runtime_error(existing.error);
	}

	if (!existing.empty()) {
		co_return "";
	}

	unsigned char bytes[32];

	if (RAND_bytes(bytes, sizeof(bytes)) != 1) {
		throw std::runtime_error("Unable to generate recovery token");
	}

	static constexpr char hex[] = "0123456789abcdef";

	std::string token;
	token.reserve(64);

	for (unsigned char byte : bytes) {
		token += hex[byte >> 4];
		token += hex[byte & 15];
	}

	const auto result = co_await db::co_query(
	    "INSERT IGNORE INTO standalone_discord_auth (user_id, token_hash) "
	    "VALUES (?, ?)",
	    {user_id, token_hash(token)}
	);

	if (!result.error.empty()) {
		throw std::runtime_error(result.error);
	}

	// Check that our token was actually inserted, rather than ignored
	// because another /link request created the account concurrently.
	const auto check = co_await db::co_query(
	    "SELECT token_hash FROM standalone_discord_auth WHERE user_id = ?",
	    {user_id}
	);

	if (!check.error.empty() || check.empty()) {
		throw std::runtime_error("Unable to verify recovery token");
	}

	if (check[0].at("token_hash") != token_hash(token)) {
		co_return "";
	}

	{
		std::unique_lock lock(linked_users_mutex);
		linked_users.insert(static_cast<uint64_t>(user_id));
	}

	co_return token;
}

void standalone::init(dpp::cluster &bot, const std::string &address, uint16_t port) {
	cluster = &bot;
	auto linked = db::query("SELECT user_id FROM standalone_discord_auth");
	{
		std::unique_lock lock(linked_users_mutex);
		for (const auto& row : linked) {
			linked_users.insert(std::stoull(row.at("user_id")));
		}
	}
	std::thread([address, port]() {
		dpp::utility::set_thread_name("app/ws-server");
		drogon::app()
			.addListener(address, port)
			.setThreadNum(2)
			.disableSigtermHandling()
			.run();
	}).detach();
}

void standalone::input(dpp::snowflake user_id, const json &payload) {
	if (!cluster || !payload.is_object() || !payload.contains("type") || !payload["type"].is_number_integer() || !payload.contains("data") || !payload["data"].is_object()) {
		return;
	}

	const int type = payload["type"].get<int>();

	if (type != 2 && type != 3 && type != 5) {
		return;
	}

	dpp::json data = payload["data"];

	if (type == 2) {
		data["type"] = 1;
	} else if (type == 3) {
		if (!data.contains("custom_id") || !data["custom_id"].is_string() || !data.contains("component_type") || !data["component_type"].is_number_integer()) {
			return;
		}
	} else {
		if (!data.contains("custom_id") || !data["custom_id"].is_string() || !data.contains("components") || !data["components"].is_array()) {
			return;
		}
	}

	const std::string id = std::to_string(next_id());

	dpp::json interaction = {
		{"id", id},
		{"application_id", cluster->me.id.str()},
		{"type", type},
		{"token", "ssod_" + std::to_string(user_id)},
		{"version", 1},
		{"channel_id", "1"},
		{"locale", "en-GB"},
		{
			"user", {
				{"id", user_id.str()},
				{"username", "Player"},
				{"discriminator", "0"},
				{"avatar", nullptr}
			}
		},
		{"data", std::move(data)}
	};

	if (type == 3) {
		std::string message_id = id;

		if (payload.contains("message_id") && payload["message_id"].is_string()) {
			message_id = payload["message_id"].get<std::string>();
		}

		interaction["message"] = {
			{"id", message_id},
			{"channel_id", "1"},
			{"content", ""},
			{"embeds", json::array()},
			{"components", json::array()}
		};
	}

	const std::string raw = interaction.dump();

	dpp::events::internal_handle_interaction(
		cluster, 0, interaction, raw, false
	);
}

void standalone::output(dpp::snowflake user_id, const json &response) {
	connection_ptr conn;

	{
		std::lock_guard lock(connections_mutex);
		auto it = connections.find(static_cast<uint64_t>(user_id));

		if (it != connections.end()) {
			conn = it->second;
		}
	}

	if (conn && conn->connected()) {
		conn->send(response.dump());
	}
}

void standalone::handleNewConnection(const drogon::HttpRequestPtr &,
					const connection_ptr &conn) {
	conn->setContext(std::make_shared<client>());
}

void standalone::handleNewMessage(const connection_ptr &conn, std::string &&message,
				const drogon::WebSocketMessageType &type) {
	if (type != drogon::WebSocketMessageType::Text) {
		return;
	}

	json payload = json::parse(message, nullptr, false);

	if (payload.is_discarded() || !payload.is_object()) {
		send_error(conn, "Invalid JSON");
		return;
	}

	auto state = conn->getContext<client>();

	if (!state) {
		return;
	}

	uint64_t user_id = 0;

	{
		std::lock_guard lock(connections_mutex);
		user_id = state->user_id;
	}

	if (!user_id) {
		if (!payload.contains("type") || payload["type"] != "auth" || !payload.contains("token") || !payload["token"].is_string() || !valid_token(payload["token"].get<std::string>())) {
			send_error(conn, "Authentication required");
			return;
		}

		{
			std::lock_guard lock(connections_mutex);
			if (state->authenticating) {
				return;
			}
			state->authenticating = true;
		}

		authenticate(conn, payload["token"].get<std::string>());
		return;
	}

	{
		std::lock_guard lock(connections_mutex);
		auto it = connections.find(user_id);
		if (it == connections.end() || it->second != conn) {
			return;
		}
	}

	input(user_id, payload);
}

void standalone::handleConnectionClosed(const connection_ptr &conn) {
	auto state = conn->getContext<client>();

	if (!state) {
		return;
	}

	std::lock_guard lock(connections_mutex);
	auto it = connections.find(state->user_id);

	if (it != connections.end() && it->second == conn) {
		connections.erase(it);
	}
}
}
