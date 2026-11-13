#pragma once

#include <dpp/dpp.h>
#include <drogon/WebSocketController.h>
#include <cstdint>
#include <string>

namespace ssod {

class standalone : public drogon::WebSocketController<standalone> {
public:
	using json = dpp::json;

	static void init(dpp::cluster& bot, const std::string& address = "127.0.0.1", uint16_t port = 8080);
	static void input(dpp::snowflake user_id, const json& interaction);
	static void output(dpp::snowflake user_id, const json& response);

	static dpp::task<std::string> link(dpp::snowflake user_id);
	static bool is_linked(dpp::snowflake user_id);
	static bool is_connected(dpp::snowflake user_id);
	static uint64_t steam_id(dpp::snowflake user_id);

	void handleNewConnection(const drogon::HttpRequestPtr& request, const drogon::WebSocketConnectionPtr& connection) override;
	void handleNewMessage(const drogon::WebSocketConnectionPtr& connection, std::string&& message, const drogon::WebSocketMessageType& type) override;
	void handleConnectionClosed(const drogon::WebSocketConnectionPtr& connection) override;

	WS_PATH_LIST_BEGIN
	WS_PATH_ADD("/api/game");
	WS_PATH_LIST_END
};

}