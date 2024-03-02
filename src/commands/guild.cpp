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
#include <ssod/commands/guild.h>
#include <ssod/game_util.h>
#include <ssod/database.h>
#include <ssod/aes.h>

dpp::slashcommand guild_command::register_command(dpp::cluster& bot) {
	return dpp::slashcommand("guild", "Create or join a guild", bot.me.id)
		.set_dm_permission(true)
		.add_option(
			dpp::command_option(dpp::co_sub_command, "create", "Create a guild")
			.add_option(dpp::command_option(dpp::co_string, "name", "Guild name to create", true))
		)
		.add_option(
			dpp::command_option(dpp::co_sub_command, "join", "Join a guild")
			.add_option(dpp::command_option(dpp::co_string, "name", "Guild name to join", true))
		)
		.add_option(
			dpp::command_option(dpp::co_sub_command, "info", "Show guild information")
			.add_option(dpp::command_option(dpp::co_string, "name", "Guild name to show", true))
		)
		.add_option(
			dpp::command_option(dpp::co_sub_command, "leave", "Leave your guild")
		);
}

void guild_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.from->creator;

	dpp::command_interaction cmd_data = event.command.get_command_interaction();
	auto subcommand = cmd_data.options[0];

	dpp::embed embed;
	embed.set_url("https://ssod.org/")
		.set_title(subcommand.name == "create" ? "Create Guild" : "Join Guild")
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR);


	if (subcommand.name == "create") {
		auto param = subcommand.options[0].value;
		std::string name = std::get<std::string>(param);
		db::transaction();
		auto rs = db::query("SELECT id FROM guilds WHERE name = ?", { name });
		if (rs.empty()) {
			db::query("INSERT INTO guilds (owner_id, name) VALUES(?, ?)", { event.command.usr.id, name });
			auto rs = db::query("SELECT id FROM guilds WHERE name = ?", { name });
			db::query("INSERT INTO guild_members (user_id, guild_id) VALUES(?, ?)", { event.command.usr.id, rs[0].at("id") });
			embed.set_description("Guild has been created:\n\n" + dpp::utility::markdown_escape(name));
		} else {
			embed.set_description("Guild already exists");
		}
		db::commit();
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	} else if (subcommand.name == "join") {
		auto param = subcommand.options[0].value;
            	std::string name = std::get<std::string>(param);
		db::transaction();
		auto g = db::query("SELECT * FROM guild_members JOIN guilds ON guild_id = guilds.id WHERE user_id = ?", { event.command.usr.id });
		if (g.empty()) {
			auto rs = db::query("SELECT id FROM guilds WHERE name = ?", { name });
			if (!rs.empty()) {
				db::query("INSERT INTO guild_members (user_id, guild_id) VALUES(?, ?)", { event.command.usr.id, rs[0].at("id") });
			} else {
				embed.set_description("Guild does not exist");
			}
			embed.set_description("You have joined the guild:\n\n" + dpp::utility::markdown_escape(name));
		} else {
			embed.set_description("You are already a member of guild:\n\n" + dpp::utility::markdown_escape(g[0].at("name")));
		}
		db::commit();
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	} else if (subcommand.name == "info") {
		auto param = subcommand.options[0].value;
            	std::string name = std::get<std::string>(param);
		embed.set_title("Guild Information");
		auto rs = db::query("SELECT id, guilds.name AS guild_name, owner_id, (SELECT COUNT(*) FROM guild_members WHERE guild_id = guilds.id) AS member_count, game_users.* FROM guilds LEFT JOIN game_users ON guilds.owner_id = game_users.user_id WHERE guilds.name = ?", { name });
		if (!rs.empty()) {
			std::string description{"# " + dpp::utility::markdown_escape(rs[0].at("guild_name")) + "\n\nMember Count: " + rs[0].at("member_count") + "\n" };
			if (!rs[0].at("name").empty())
			description += "\nOwner: **" + dpp::utility::markdown_escape(rs[0].at("name")) + "**";
			embed.set_description(description);
		} else {
			embed.set_description("Guild does not exist");
		}
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	} else if (subcommand.name == "leave") {
		db::transaction();
		embed.set_title("Leave Guild");
		auto g = db::query("SELECT * FROM guild_members JOIN guilds ON guild_id = guilds.id WHERE user_id = ?", { event.command.usr.id });
		if (g.empty()) {
			embed.set_description("You are not a member of a guild.");
		} else {
			db::query("DELETE FROM guild_members WHERE user_id = ?", { event.command.usr.id });
			embed.set_description("You have left your current guild: " + dpp::utility::markdown_escape(g[0].at("name")));
		}
		db::commit();
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	}
}
