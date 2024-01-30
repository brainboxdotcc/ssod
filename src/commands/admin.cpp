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
#include <ssod/ssod.h>
#include <ssod/database.h>
#include <ssod/commands/admin.h>
#include <ssod/database.h>
#include <ssod/game_date.h>
#include <ssod/game_player.h>

static void autocomplete(dpp::cluster& bot, const dpp::autocomplete_t& event, const std::string& uservalue) {
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

	dpp::command_option duration(dpp::co_integer, "duration", "How long to mute the user for");
	duration
		.add_choice(dpp::command_option_choice("One Minute", (int64_t)60))
		.add_choice(dpp::command_option_choice("Ten Minutes", (int64_t)600))
		.add_choice(dpp::command_option_choice("One Hour", (int64_t)3600))
		.add_choice(dpp::command_option_choice("One Day", (int64_t)86400))
		.add_choice(dpp::command_option_choice("One Week", (int64_t)604800))
		.add_choice(dpp::command_option_choice("Four Weeks", (int64_t)604800*4));

	return dpp::slashcommand("admin", "Game Moderation Commands", bot.me.id)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "teleport", "Teleport yourself to new location ID")
			.add_option(dpp::command_option(dpp::co_integer, "location", "Location to teleport to", true))
		)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "mute", "Mute a user")
			.add_option(dpp::command_option(dpp::co_string, "user", "User to mute", true).set_auto_complete(true))
			.add_option(duration)
		)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "pin", "Pin a user")
			.add_option(dpp::command_option(dpp::co_string, "user", "User to pin", true).set_auto_complete(true))
			.add_option(duration)
		)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "reset", "Reset a user")
			.add_option(dpp::command_option(dpp::co_string, "user", "User to reset", true).set_auto_complete(true))
		);
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
		auto check = db::query("SELECT secure_id, id FROM game_locations WHERE id = ? OR secure_id = ?", {location, location});
		if (check.empty()) {
			event.reply(dpp::message("Location " + std::to_string(location) + " does not exist.").set_flags(dpp::m_ephemeral));	
			return;
		}
		db::query("UPDATE game_users SET paragraph = ? WHERE user_id = ?", {check[0].at("id"), event.command.usr.id});
		player p(event.command.usr.id);
		p.save(event.command.usr.id);
		p.state = state_play;
		p.after_fragment = 0;
		p.combatant = {};
		update_live_player(event, p);
		event.reply(dpp::message("You have been teleported to location " + check[0].at("id")).set_flags(dpp::m_ephemeral));
		bot.log(dpp::ll_info, "ADMIN TELEPORT " + event.command.usr.global_name + " to " + check[0].at("id"));
	}
	if (subcommand.name == "mute" || subcommand.name == "pin") {
		std::string user = std::get<std::string>(subcommand.options[0].value);
		int64_t duration = std::get<int64_t>(subcommand.options[1].value);
		std::string field{subcommand.name == "mute" ? "muted" : "pinned"};
		db::query("UPDATE game_users SET " + field + " = ? WHERE name = ?", { duration + time(nullptr), user});
		event.reply(dpp::message(user + " has been " + field + " until " + dpp::utility::timestamp(duration + time(nullptr))).set_flags(dpp::m_ephemeral));
		bot.log(dpp::ll_info, "ADMIN " + dpp::uppercase(subcommand.name) + " by " + event.command.usr.global_name + " -> " + user);
	}
	if (subcommand.name == "reset") {
		std::string user = std::get<std::string>(subcommand.options[0].value);
		auto rs = db::query("SELECT id FROM game_users WHERE name = ?", {user});
		uint64_t id = atoll(rs[0].at("id").c_str());
		// Get the backup copy of the user
		player p(id, true);
		// Write the backup copy to the live copy
		p.save(id, false);
		event.reply(dpp::message(user + " has been reset.").set_flags(dpp::m_ephemeral));
		bot.log(dpp::ll_info, "ADMIN RESET by " + event.command.usr.global_name + " -> " + user);
	}
}
