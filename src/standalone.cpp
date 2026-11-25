#include <ssod/standalone.h>
#include <ssod/database.h>
#include <dpp/dispatcher.h>
#include <openssl/sha.h>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <openssl/rand.h>
#include <shared_mutex>
#include <unordered_set>
#include <stdexcept>
#include <ssod/aes.h>
#include <ssod/game_player.h>

namespace ssod {

using json = nlohmann::json;
using connection_ptr = drogon::WebSocketConnectionPtr;

enum app_source_t {
	src_web,
	src_android,
	src_apple,
	src_steam,
	src_windows,
};

struct client {
	uint64_t user_id{};
	uint64_t steam_id{};
	bool authenticating{};
	bool started{};
	std::chrono::steady_clock::time_point last_interaction{std::chrono::steady_clock::now()};
	double interaction_tokens{10.0};
	app_source_t source{src_web};
};

std::mutex connections_mutex;
std::unordered_map<uint64_t, connection_ptr> connections;
dpp::cluster *cluster{};
std::atomic<uint64_t> serial{0};

std::shared_mutex linked_users_mutex;
std::unordered_set<uint64_t> linked_users;

const char *source_name(app_source_t source) {
	switch (source) {
		case src_web: return "web";
		case src_android: return "android";
		case src_apple: return "ios";
		case src_steam: return "steam";
		case src_windows: return "windows";
	}
	return "unknown";
}

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

void finish_authentication(const connection_ptr &conn, uint64_t id, bool ads_enabled, bool premium) {
	if (id == 0) {
		send_error(conn, "Invalid app user ID");
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
		get_bot().log(dpp::ll_info, "App login from user id: " + std::to_string(id) + "; source: " + source_name(state->source));
	}

	if (conn->connected()) {
		conn->send(json{
			{"type", "authenticated"},
			{"user_id", std::to_string(id)},
			{"ads_enabled", ads_enabled},
			{"premium", premium}
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

void check_premium(const connection_ptr &conn, uint64_t id) {
	bool mobile = false;

	{
		std::lock_guard lock(connections_mutex);
		auto state = conn->getContext<client>();

		if (!state || !state->authenticating || !conn->connected()) {
			return;
		}

		mobile = state->source == src_android || state->source == src_apple;
	}

	if (id == 0) {
		finish_authentication(conn, id, false, false);
		return;
	}

	db::query_callback(
		"SELECT user_id FROM premium_credits WHERE user_id = ? AND active = 1 LIMIT 1", {id},
		[conn, id, mobile](const db::resultset result) {
			const bool premium = result.error.empty() && !result.empty();
			const bool ads_enabled = result.error.empty() && mobile && !premium;
			finish_authentication(conn, id, ads_enabled, premium);
		}
	);
}

void authenticate(const connection_ptr &conn, const std::string &token, bool recover = false) {
	const std::string hash = token_hash(token);

	db::query_callback("SELECT user_id FROM standalone_discord_auth WHERE token_hash = ?", {hash},
	[conn, hash, recover](const db::resultset linked) {
		if (!linked.error.empty()) {
			send_error(conn, "Authentication unavailable");
			return;
		}

		if (!linked.empty()) {
			const uint64_t id = linked[0].number<uint64_t>("user_id");

			db::query_callback("SELECT user_id FROM standalone_auth WHERE user_id = ?", {id},
			[conn, id](const db::resultset existing) {
				if (!existing.error.empty()) {
					send_error(conn, "Authentication unavailable");
					return;
				}

				if (!existing.empty()) {
					send_error(conn, "This Discord account is already associated with another app account");
					return;
				}

				check_premium(conn, id);
			});
			return;
		}

		if (recover) {
			db::query_callback("SELECT user_id FROM standalone_auth WHERE token_hash = ?", {hash},
			[conn](const db::resultset result) {
				if (!result.error.empty()) {
					send_error(conn, "Authentication unavailable");
					return;
				}

				if (result.empty()) {
					send_error(conn, "Recovery token not recognised");
					return;
				}

				check_premium(conn, result[0].number<uint64_t>("user_id"));
			});
			return;
		}

		db::query_callback("INSERT INTO standalone_auth (token_hash) VALUES (?) ON DUPLICATE KEY UPDATE user_id = LAST_INSERT_ID(user_id)", {hash},
		[conn](const db::resultset result) {
			if (!result.error.empty()) {
				send_error(conn, "Authentication unavailable");
				return;
			}

			check_premium(conn, result.insert_id);
		});
	});
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
		"SELECT user_id FROM standalone_discord_auth WHERE user_id = ? "
		"UNION ALL "
		"SELECT user_id FROM standalone_auth WHERE user_id = ?",
		{user_id, user_id}
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

void sign_custom_ids(json &value) {
	if (value.is_array()) {
		for (auto &child : value) {
			sign_custom_ids(child);
		}
		return;
	}

	if (!value.is_object()) {
		return;
	}

	for (auto &[key, child] : value.items()) {
		if (key == "custom_id" && child.is_string()) {
			child = security::sign(child.get<std::string>());
		} else {
			sign_custom_ids(child);
		}
	}
}

bool verify_custom_ids(json &value) {
	if (value.is_array()) {
		for (auto &child : value) {
			if (!verify_custom_ids(child)) {
				return false;
			}
		}
		return true;
	}

	if (!value.is_object()) {
		return true;
	}

	for (auto &[key, child] : value.items()) {
		if (key == "custom_id") {
			if (!child.is_string()) {
				return false;
			}

			const std::string unsigned_id = security::verify_signed(child.get<std::string>());

			if (unsigned_id.empty()) {
				return false;
			}

			child = unsigned_id;
		} else if (!verify_custom_ids(child)) {
			return false;
		}
	}

	return true;
}

void standalone::input(dpp::snowflake user_id, const json &payload) {
	if (!cluster || !payload.is_object() || !payload.contains("type") || !payload["type"].is_number_integer() || !payload.contains("data") || !payload["data"].is_object()) {
		return;
	}

	const int type = payload["type"].get<int>();

	if (type != 2 && type != 3 && type != 4 && type != 5) {
		return;
	}

	dpp::json data = payload["data"];

	if (type == 2 || type == 4) {
		data["type"] = 1;
	} else if (type == 3) {
		if (!data.contains("custom_id") || !data["custom_id"].is_string() || !data.contains("component_type") || !data["component_type"].is_number_integer()) {
			return;
		}

		if (!verify_custom_ids(data)) {
			return;
		}
	} else {
		if (!data.contains("custom_id") || !data["custom_id"].is_string() || !data.contains("components") || !data["components"].is_array()) {
			return;
		}

		if (!verify_custom_ids(data)) {
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
		json signed_response = response;
		sign_custom_ids(signed_response);
		conn->send(signed_response.dump());
	}
}

void standalone::output_message(dpp::snowflake user_id, const dpp::message &m) {
	ssod::standalone::output(user_id, {{"type", dpp::ir_update_message}, {"data", json::parse(m.build_json())}});
}


void standalone::handleNewConnection(const drogon::HttpRequestPtr &, const connection_ptr &conn) {
	conn->setContext(std::make_shared<client>());
	std::weak_ptr<drogon::WebSocketConnection> weak_conn = conn;
	drogon::app().getLoop()->runAfter(10.0, [weak_conn]() {
		if (auto pending = weak_conn.lock()) {
			auto state = pending->getContext<client>();
			bool unauthenticated = false;
			{
				std::lock_guard lock(connections_mutex);
				unauthenticated = state && !state->user_id;
			}
			if (unauthenticated && pending->connected()) {
				pending->shutdown();
			}
		}
	});
}

void standalone::handleNewMessage(const connection_ptr &conn, std::string &&message, const drogon::WebSocketMessageType &type) {
	if (type != drogon::WebSocketMessageType::Text) {
		return;
	}

	if (message.size() > 16384) {
		conn->shutdown();
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

		uint64_t steam_id = 0;

		if (payload.contains("steam_id") && payload["steam_id"].is_string()) {
			const std::string value = payload["steam_id"].get<std::string>();
			try {
				steam_id = std::stoull(value);
			} catch (const std::exception &) {
				steam_id = 0;
			}
		}

		app_source_t source{src_web};
		if (payload.contains("platform") && payload["platform"].is_string()) {
			const std::string platform = payload["platform"].get<std::string>();
			if (platform == "android") {
				source = src_android;
			} else if (platform == "ios") {
				source = src_apple;
			} else if (platform == "steam") {
				source = src_steam;
			} else if (platform == "windows") {
				source = src_windows;
			}
		}

		{
			std::lock_guard lock(connections_mutex);
			if (state->authenticating) {
				return;
			}
			state->steam_id = steam_id;
			state->authenticating = true;
			state->source = source;
		}

		authenticate(conn, payload["token"].get<std::string>(), payload.contains("recover") && payload["recover"].is_boolean() && payload["recover"].get<bool>());
		return;
	}

	{
		std::lock_guard lock(connections_mutex);
		auto it = connections.find(user_id);
		if (it == connections.end() || it->second != conn) {
			return;
		}
		const auto now = std::chrono::steady_clock::now();
		const double elapsed = std::chrono::duration<double>(now - state->last_interaction).count();
		state->interaction_tokens = std::min(10.0, state->interaction_tokens + elapsed * 5.0);
		state->last_interaction = now;
		if (state->interaction_tokens < 1.0) {
			return;
		}
		state->interaction_tokens -= 1.0;
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

uint64_t standalone::steam_id(dpp::snowflake user_id) {
	std::lock_guard lock(connections_mutex);

	auto it = connections.find(static_cast<uint64_t>(user_id));

	if (it == connections.end() || !it->second->connected()) {
		return 0;
	}

	auto state = it->second->getContext<client>();

	return state ? state->steam_id : 0;
}

void standalone::refresh_chat(uint32_t paragraph, dpp::snowflake sender, const std::string& message, const std::string& event_type) {
	std::vector<dpp::snowflake> recipients;

	{
		std::lock_guard lock(connections_mutex);

		for (const auto& [user_id, conn] : connections) {
			if (conn->connected()) {
				recipients.push_back(user_id);
			}
		}
	}

	for (dpp::snowflake user_id : recipients) {
		dpp::interaction_create_t event(cluster, 0, "");
		event.command.usr.id = user_id;

		player p = get_live_player(event, false);

		if (p.state == state_play && p.paragraph == paragraph) {
			standalone::output(user_id, {
				{"type", "chat_refresh"},
				{"paragraph", paragraph},
				{"event_type", event_type},
				{"user_id", sender.str()},
				{"message", message}
			});
		}
	}
}

bool standalone::first_start(dpp::snowflake user_id) {
	std::lock_guard lock(connections_mutex);

	auto it = connections.find(static_cast<uint64_t>(user_id));
	if (it == connections.end()) {
		return true;
	}

	auto state = it->second->getContext<client>();
	if (!state) {
		return true;
	}

	if (state->started) {
		return false;
	}

	state->started = true;
	return true;
}

}
