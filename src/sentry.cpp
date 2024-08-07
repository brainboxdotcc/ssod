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

#include <mutex>
#include <deque>
#include <chrono>
#include <dpp/dpp.h>
#include <ssod/ssod.h>
#include <ssod/sentry.h>
#include <ssod/config.h>
#include <fmt/format.h>
#include <sentry.h>

namespace sentry {

	sentry_transport_t *transport = nullptr;
	void *transport_state = nullptr;
	dpp::cluster *bot = nullptr;
	std::mutex sentry_mutex{};
	void* sentry_transaction;

	/**
	 * @brief Custom sentry transport, writes envelopes for posting later
	 * on a separate thread.
	 */
	void custom_transport(sentry_envelope_t *envelope, void *state) {
		size_t size_out{0};
		char *env_str = sentry_envelope_serialize(envelope, &size_out);
		std::ofstream envelope_file;
		envelope_file.open(fmt::format("/tmp/.sentry_envelope_{}.json", dpp::utility::time_f()), std::ios::out);
		envelope_file << env_str;
		envelope_file.close();
		sentry_free(env_str);
		sentry_envelope_free(envelope);
	}

	bool init(dpp::cluster &creator) {
		bot = &creator;

		std::string dsn = config::get("sentry_dsn");
		std::string env = config::get("environment");
		double sample_rate = config::get("sentry_sample_rate").get<double>();
		sentry_options_t *options = sentry_options_new();
		sentry_options_set_dsn(options, dsn.c_str());
		sentry_options_set_database_path(options, ".sentry-native");
		sentry_options_set_release(options, SSOD_VERSION.data());
		sentry_options_set_debug(options, 0);
		sentry_options_set_environment(options, env.c_str());
		sentry_options_set_traces_sample_rate(options, sample_rate);
		transport = sentry_transport_new(custom_transport);
		sentry_transport_set_state(transport, transport_state);
		sentry_options_set_transport(options, transport);

		sentry_init(options);

		sentry_set_tag("cluster", std::to_string(creator.cluster_id).c_str());

		return true;
	}

	void *register_transaction_type(const std::string &transaction_name, const std::string &transaction_operation) {
		std::lock_guard<std::mutex> lock(sentry_mutex);
		return sentry_transaction_context_new(
			transaction_name.c_str(),
			transaction_operation.c_str()
		);
	}

	void *span(void *tx, const std::string &query) {
		if (tx) {
			std::lock_guard<std::mutex> lock(sentry_mutex);
			return sentry_transaction_start_child((sentry_transaction_t *) tx, "db.sql.query", query.c_str());
		}
		return nullptr;
	}

	void set_span_status(void *tx, sentry_status s) {
		if (tx) {
			std::lock_guard<std::mutex> lock(sentry_mutex);
			sentry_span_t *span = (sentry_span_t *) tx;
			sentry_span_status_t status = (sentry_span_status_t) s;
			sentry_span_set_status(span, status);
		}
	}

	void log_catch(const std::string &exception_type, const std::string &what) {
		std::lock_guard<std::mutex> lock(sentry_mutex);
		sentry_value_t exc = sentry_value_new_exception(exception_type.c_str(), what.c_str());
		sentry_value_set_stacktrace(exc, NULL, 0);
		sentry_value_t event = sentry_value_new_event();
		sentry_event_add_exception(event, exc);
		sentry_capture_event(event);
	}

	void set_user(const dpp::user &user, uint64_t guild_id) {
		std::lock_guard<std::mutex> lock(sentry_mutex);
		sentry_value_t u = sentry_value_new_object();
		sentry_value_set_by_key(u, "id", sentry_value_new_string(user.id.str().c_str()));
		sentry_value_set_by_key(u, "username", sentry_value_new_string(user.format_username().c_str()));
		sentry_value_set_by_key(u, "guild_id", sentry_value_new_string(std::to_string(guild_id).c_str()));
		sentry_set_user(u);
	}

	void unset_user() {
		std::lock_guard<std::mutex> lock(sentry_mutex);
		sentry_remove_user();
	}

	void log_message_event(const std::string &logger, const std::string &message, sentry_level_t level) {
		std::lock_guard<std::mutex> lock(sentry_mutex);
		sentry_value_t event = sentry_value_new_message_event(level, logger.c_str(), message.c_str());
		sentry_event_value_add_stacktrace(event, NULL, 0);
		sentry_capture_event(event);
	}

	void log_error(const std::string &logger, const std::string &message) {
		log_message_event(logger.c_str(), message.c_str(), SENTRY_LEVEL_ERROR);
	}

	void log_warning(const std::string &logger, const std::string &message) {
		log_message_event(logger.c_str(), message.c_str(), SENTRY_LEVEL_WARNING);
	}

	void log_critical(const std::string &logger, const std::string &message) {
		log_message_event(logger.c_str(), message.c_str(), SENTRY_LEVEL_FATAL);
	}

	void end_span(void *span) {
		if (span) {
			std::lock_guard<std::mutex> lock(sentry_mutex);
			sentry_span_finish((sentry_span_t *) span);
		}
	}

	std::string version() {
		return sentry_sdk_version();
	}

	void *start_transaction(void *tx_ctx) {
		if (!tx_ctx) {
			return nullptr;
		}
		std::lock_guard<std::mutex> lock(sentry_mutex);
		sentry_transaction_t *tx = sentry_transaction_start((sentry_transaction_context_t *) tx_ctx, sentry_value_new_null());
		return tx;
	}

	void make_new_transaction(const std::string &command) {
		{
			std::lock_guard<std::mutex> lock(sentry_mutex);
			if (sentry_transaction != nullptr) {
				return;
			}
		}
		sentry_transaction = start_transaction(sentry::register_transaction_type(command.c_str(), "db"));
	}

	void end_user_transaction() {
		end_transaction(sentry_transaction);
		sentry_transaction = nullptr;
	}

	void* get_user_transaction() {
		std::lock_guard<std::mutex> lock(sentry_mutex);
		return sentry_transaction;
	}

	void end_transaction(void* opaque_transaction) {
		if (opaque_transaction) {
			std::lock_guard<std::mutex> lock(sentry_mutex);
			sentry_transaction_finish((sentry_transaction_t*)opaque_transaction);
		}
	}

	void close() {
		sentry_close();
	}
};
