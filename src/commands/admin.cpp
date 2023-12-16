/************************************************************************************
 * 
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023 Craig Edwards <support@sporks.gg>
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
#include <ssod/ssod.h>
#include <ssod/database.h>
#include <ssod/commands/admin.h>
#include <ssod/database.h>
#include <ssod/sentry.h>
#include <ssod/game_date.h>

void autocomplete(dpp::cluster& bot, const dpp::autocomplete_t& event, const std::string& uservalue) {
	auto rs = db::query("SELECT lower(name) AS name FROM game_users WHERE name LIKE ?", {uservalue + "%"});
	dpp::interaction_response ir(dpp::ir_autocomplete_reply);
	for (const auto& r : rs) {
		ir.add_autocomplete_choice(dpp::command_option_choice(r.at("name"), r.at("name")));
	}
	bot.interaction_response_create(event.command.id, event.command.token, ir);
}

dpp::slashcommand admin_command::register_command(dpp::cluster& bot) {
	bot.on_autocomplete([&bot](const dpp::autocomplete_t & event) {
		for (auto & opt : event.options) {
			if (opt.name != "mute" && opt.name != "pin" && opt.name != "reset") {
				return;
			}
			json j = json::parse(event.raw_event);
			j = j["d"];
			if (j.at("data").contains("options")) {
				json& choices = j["data"]["options"][0]["options"][0];
				if (choices.at("focused").get<bool>() == true && choices.at("name").get<std::string>() == "user") {
					autocomplete(bot, event, choices.at("value").get<std::string>());
				}
			}
		}
	});

	return dpp::slashcommand("admin", "Game Moderation Commands", bot.me.id)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "teleport", "Teleport yourself to new location ID")
			.add_option(dpp::command_option(dpp::co_integer, "location", "Location to teleport to", true))
		)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "mute", "Mute a user")
			.add_option(dpp::command_option(dpp::co_string, "user", "User to mute", true).set_auto_complete(true))
		)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "pin", "Pin a user")
			.add_option(dpp::command_option(dpp::co_string, "user", "User to pin", true).set_auto_complete(true))
		)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "reset", "Reset a user")
			.add_option(dpp::command_option(dpp::co_string, "user", "User to reset", true).set_auto_complete(true))
		)
		;

}

void admin_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.from->creator;
	auto rs = db::query("SELECT * FROM game_admins WHERE user_id = ?", {event.command.usr.id});
	if (rs.empty()) {
		event.reply("This command is for game admins only");
		return;
	}

	dpp::command_interaction cmd_data = event.command.get_command_interaction();
	auto subcommand = cmd_data.options[0];

	if (subcommand.name == "teleport") {
		int64_t location = std::get<int64_t>(subcommand.options[0].value);
		auto check = db::query("SELECT id FROM game_locations WHERE id = ?", {location});
		if (check.empty()) {
			event.reply(dpp::message("Location " + std::to_string(location) + " does not exist.").set_flags(dpp::m_ephemeral));	
			return;
		}
		db::query("UPDATE game_users SET paragraph = ? WHERE user_id = ?", {location, event.command.usr.id});
		event.reply(dpp::message("You have been teleported to location " + std::to_string(location)).set_flags(dpp::m_ephemeral));
		bot.log(dpp::ll_info, "ADMIN TELEPORT " + event.command.usr.global_name + " to " + std::to_string(location));
	}
}
