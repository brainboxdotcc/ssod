#pragma once

#include <dpp/dpp.h>
#include <drogon/WebSocketController.h>
#include <cstdint>
#include <string>

namespace ssod {

/**
 * @brief Manages websocket connections between the game server and app clients
 *
 * Provides authentication, interaction dispatch, message delivery and account
 * linking between Discord and the app
 *
 * Connected clients use the existing Discord interaction protocol, allowing
 * game commands and components to be processed by the same handlers regardless
 * of whether the player is using Discord or the app
 */
class standalone : public drogon::WebSocketController<standalone> {
public:
	using json = dpp::json;

	/**
	 * @brief Initialise the websocket server and load linked Discord accounts
	 * @param bot The DPP cluster used to dispatch game interactions
	 * @param address Network address on which to listen
	 * @param port TCP port on which to listen
	 */
	static void init(dpp::cluster& bot, const std::string& address = "127.0.0.1", uint16_t port = 9090);

	/**
	 * @brief Dispatch an app interaction through the existing DPP handlers
	 * @param user_id The authenticated player's user ID
	 * @param interaction JSON payload representing a Discord interaction
	 */
	static void input(dpp::snowflake user_id, const json& interaction);

	/**
	 * @brief Send a JSON response directly to a connected app client
	 * @param user_id The recipient's user ID
	 * @param response JSON payload to send over the websocket
	 *
	 * Does nothing if the player is not connected
	 */
	static void output(dpp::snowflake user_id, const json& response);

	/**
	 * @brief Send a Discord message directly to a connected app client
	 * @param user_id The recipient's user ID
	 * @param m The DPP message to serialise and deliver
	 *
	 * Wraps the message in a Discord interaction update response before
	 * sending it over the websocket. Does nothing if the player is not connected
	 */
	static void output_message(dpp::snowflake user_id, const dpp::message &m);

	/**
	 * @brief Link a Discord account for use through the app
	 * @param user_id The Discord user's snowflake ID
	 * @return A newly generated recovery token, or an empty string if already linked
	 *
	 * Stores the token hash in the database and updates the in-memory
	 * linked-user cache
	 */
	static dpp::task<std::string> link(dpp::snowflake user_id);

	/**
	 * @brief Check whether a Discord account is linked to the app
	 * @param user_id The Discord user's snowflake ID
	 * @return True if the account is linked
	 */
	static bool is_linked(dpp::snowflake user_id);

	/**
	 * @brief Check whether a player has an active app websocket connection
	 * @param user_id The player's user ID
	 * @return True if the player is currently connected and authenticated
	 */
	static bool is_connected(dpp::snowflake user_id);

	/**
	 * @brief Retrieve the Steam ID associated with a connected app session
	 * @param user_id The player's user ID
	 * @return The SteamID64, or zero if unavailable or disconnected
	 */
	static uint64_t steam_id(dpp::snowflake user_id);

	/**
	 * @brief Force all connected app users on this paragraph to refresh their chats and pick up new chat content
	 * @param paragraph paragraph id
	 * @param sender sender snowflake id
	 * @param message message raw text
	 * @param event_type chat event type
	 */
	static void refresh_chat(uint32_t paragraph, dpp::snowflake sender, const std::string& message, const std::string& event_type);

	/**
	 * @brief Handle a newly established websocket connection
	 * @param request The HTTP request initiating the connection
	 * @param connection The newly established websocket connection
	 *
	 * Initialises per-connection state pending authentication
	 */
	void handleNewConnection(const drogon::HttpRequestPtr& request, const drogon::WebSocketConnectionPtr& connection) override;

	/**
	 * @brief Process an incoming websocket message
	 * @param connection The originating websocket connection
	 * @param message The received message payload
	 * @param type The websocket message type
	 *
	 * Handles authentication for unauthenticated connections and forwards
	 * validated interactions from authenticated players to DPP
	 */
	void handleNewMessage(const drogon::WebSocketConnectionPtr& connection, std::string&& message, const drogon::WebSocketMessageType& type) override;

	/**
	 * @brief Handle a closed websocket connection
	 * @param connection The connection that has closed
	 *
	 * Removes the connection from the active-player map if it is still
	 * the registered connection for that player
	 */
	void handleConnectionClosed(const drogon::WebSocketConnectionPtr& connection) override;

	/**
	 * @brief Returns true if the user has already started
	 * @param user_id
	 * @return
	 */
	static bool first_start(dpp::snowflake user_id);

	WS_PATH_LIST_BEGIN
	WS_PATH_ADD("/api/game");
	WS_PATH_LIST_END
};

}