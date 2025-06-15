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
#include <string>
#include <ssod/game.h>
#include <ssod/game_player.h>
#include <ssod/database.h>
#include <ssod/config.h>
#include <ssod/npc.h>

using namespace i18n;

dpp::task<bool> npc_chat(const dpp::interaction_create_t& event, player& p, const std::string& message, dpp::cluster& bot) {
	auto npcs = co_await db::co_query("SELECT * FROM vendor_ai_lore WHERE location_id = ?", {p.paragraph});
	if (npcs.empty()) {
		co_return false;
	}

	db::row selected_npc;
	bool chosen = true;
	for (const auto& npc : npcs) {
		std::string npc_flags = npc.at("flags");
		if (npc_flags.empty()) {
			selected_npc = npc;
			chosen = true;
			break;
		}
		auto flags = dpp::utility::tokenize(npc_flags, ",");
		bool skip = false;
		for (const auto& flag : flags) {
			if (flag.substr(0, 1) == "!" && p.has_flag(flag.substr(1, flag.length() - 1))) {
				skip = true;
			}
			if (flag.substr(0, 1) != "!" && !p.has_flag(flag)) {
				skip = true;
			}
		}
		if (!skip) {
			selected_npc = npc;
			chosen = true;
			break;
		}
	}
	if (!chosen) {
		co_return false;
	}

	auto& config = config::get("npc_chat");

	/**
	 * Dispatches an AI dialogue request to the NPC chat backend API.
	 *
	 * This function constructs a JSON payload including:
	 *  - `message`: The player's raw input.
	 *  - `discord_id`: Used for per-user logging and moderation tracking.
	 *  - `npc_lore`: The vendor or NPC's static backstory (e.g. "sells armour in Larton"). Their name
	 *    will be appended automatically.
	 *  - `npc_rag`: Optional context retrieved dynamically via a RAG (Retrieval-Augmented Generation)
	 *    system powered by Manticore, using full-text lore chunks tied to the current world state.
	 *    Even if no explicit text is provided, the player's message will still be used to query
	 *    Manticore for relevant lore snippets.
	 *
	 * The backend (Laravel) merges these fields into a structured prompt.
	 *  - It then validates the output with a separate prompt acting as a strict filter to remove off-topic
	 *    or policy-violating responses.
	 *  - Inference is routed through a two-stage pipeline:
	 *    → Primary: Free Cloudflare AI (LLaMA 3.1 8B) if the daily 10k neuron budget allows.
	 *    → Fallback: Local mistral-7b-instruct via llama.cpp when Cloudflare usage is exhausted or fails.
	 *
	 * The returned reply is truncated to 512 characters to fit the DB field limit.
	 */
	bot.request(
		config.at("url"),
		dpp::m_post,
		[&bot, p, event, selected_npc](const auto& response) {
			try {
				const json reply = json::parse(response.body);

				if (!reply.contains("reply")) {
					std::string error = reply.at("message");
					bot.log(dpp::ll_warning, "NPC Chat API error: " + error);
					return;
				}
				std::string npc_reply = reply.at("reply");
				db::query(
					"INSERT INTO game_chat_events (event_type, npc_id, location_id, message) VALUES(?,?,?,?)",
					{"chat", selected_npc.at("id"), p.paragraph, dpp::utility::utf8substr(npc_reply, 0, 512)}
				);
				bot.log(dpp::ll_info, "NPC reply: " + selected_npc.at("name") + " -> " + npc_reply);
				continue_game(event, p, true).sync_wait();
			}
			catch (const std::exception& e) {
				bot.log(dpp::ll_warning, "NPC Chat API error: " + std::string(e.what()) + ": status=" + std::to_string(response.status) + ": " + response.body);
			}
		}, json({
				{"message", message},
				{"discord_id", event.command.usr.id.str()},
				{"npc_lore",
					selected_npc.at("backstory") +
					"\nNPC's Name: " + selected_npc.at("name") +
					"\nPlayer's Race/Species: " + std::string(::race(p.race)) +
					"\nPlayer's Profession/Class: " + std::string(::profession(p.profession)) +
					"\nPlayer's Name: " + std::string(p.get_level() >= 15 ? p.name : "Not known") +
					"\nPlayer is " + (p.notoriety > 10 ? "INFAMOUS" : (p.get_level() >= 15 ? "FAMOUS" : "AN UNKNOWN"))
				},
				{"npc_rag", selected_npc.at("rag")}}
		).dump(),
		"application/json",
		{{ "X-API-Key", config.at("key")}}
	);

	co_return true;
}