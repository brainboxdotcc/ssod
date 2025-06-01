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
#include <fmt/format.h>
#include <ssod/config.h>
#include <ssod/neutrino_api.h>

using namespace i18n;

dpp::slashcommand guild_command::register_command(dpp::cluster& bot) {
	return tr(dpp::slashcommand("cmd_guild", "HELP_GUILD_DESC", bot.me.id)
		.set_dm_permission(true)
		.set_interaction_contexts({dpp::itc_guild, dpp::itc_bot_dm, dpp::itc_private_channel})
		.add_option(
			dpp::command_option(dpp::co_sub_command, "opt_create", "G_CREATE_DESC")
			.add_option(dpp::command_option(dpp::co_string, "opt_name", "G_NAME_DESC", true))
		)
		.add_option(
			dpp::command_option(dpp::co_sub_command, "opt_join", "G_JOIN_DESC")
			.add_option(dpp::command_option(dpp::co_string, "opt_name", "G_JOIN_NAME_DESC", true))
		)
		.add_option(
			dpp::command_option(dpp::co_sub_command, "opt_rename", "G_RENAME_DESC")
			.add_option(dpp::command_option(dpp::co_string, "opt_name", "G_RENAME_NAME_DESC", true))
		)
		.add_option(
			dpp::command_option(dpp::co_sub_command, "cmd_info", "G_INFO_DESC")
			.add_option(dpp::command_option(dpp::co_string, "opt_name", "G_INFO_NAME_DESC", true))
		)
		.add_option(
			dpp::command_option(dpp::co_sub_command, "opt_leave", "G_LEAVE_DESC")
		));
}

dpp::task<void> guild_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.owner;

	dpp::command_interaction cmd_data = event.command.get_command_interaction();
	auto subcommand = cmd_data.options[0];

	dpp::embed embed;
	embed.set_url("https://ssod.org/")
		.set_title(tr(subcommand.name == "create" ? "CREATE_GUILD" : "JOIN_GUILD", event))
		.set_footer(dpp::embed_footer{ 
			.text = tr("REQUESTED_BY", event, event.command.usr.format_username()),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR);


	if (subcommand.name == "create") {
		auto param = subcommand.options[0].value;
		std::string guild_name = std::get<std::string>(param);
		neutrino swear_check(event.owner, config::get("neutrino_user"), config::get("neutrino_password"));
		swear_filter_t swear_filter = co_await swear_check.co_contains_bad_word(guild_name);
		if (!swear_filter.clean) {
			bot.log(dpp::ll_warning, "Potty-mouth guild name " + guild_name + " censored for id " + event.command.usr.id.str());
			guild_name = swear_filter.censored_content;
		}
		auto g = co_await db::co_query("SELECT * FROM guild_members JOIN guilds ON guild_id = guilds.id WHERE user_id = ?", { event.command.usr.id });
		if (g.empty()) {
			auto rs = co_await db::co_query("SELECT id FROM guilds WHERE name = ?", { guild_name });
			if (rs.empty()) {
				co_await db::co_query("INSERT INTO guilds (owner_id, name) VALUES(?, ?)", { event.command.usr.id, guild_name });
				auto rs = co_await db::co_query("SELECT id FROM guilds WHERE name = ?", { guild_name });
				co_await db::co_query("INSERT INTO guild_members (user_id, guild_id) VALUES(?, ?)", { event.command.usr.id, rs[0].at("id") });
				embed.set_description(tr("GUILD_CREATED", event) + "\n\n" + dpp::utility::markdown_escape(guild_name));
			} else {
				embed.set_description(tr("GUILD_EXISTS", event));
			}
		} else {
			embed.set_description(tr("GUILD_ALREADY_MEMBER", event) + "\n\n" + dpp::utility::markdown_escape(g[0].at("name")));
		}
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	} else if (subcommand.name == "join") {
		auto param = subcommand.options[0].value;
            	std::string guild_name = std::get<std::string>(param);
		auto g = co_await db::co_query("SELECT * FROM guild_members JOIN guilds ON guild_id = guilds.id WHERE user_id = ?", { event.command.usr.id });
		if (g.empty()) {
			auto rs = co_await db::co_query("SELECT id FROM guilds WHERE name = ?", { guild_name });
			if (!rs.empty()) {
				co_await db::co_query("INSERT INTO guild_members (user_id, guild_id) VALUES(?, ?)", { event.command.usr.id, rs[0].at("id") });
			} else {
				embed.set_description(tr("NO_SUCH_GUILD", event));
			}
			embed.set_description(tr("JOINED_GUILD", event) + "\n\n" + dpp::utility::markdown_escape(guild_name));
		} else {
			embed.set_description(tr("GUILD_ALREADY_MEMBER", event) + "\n\n" + dpp::utility::markdown_escape(g[0].at("name")));
		}
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	} else if (subcommand.name == "info") {
		auto param = subcommand.options[0].value;
            	std::string guild_name = std::get<std::string>(param);
		embed.set_title(tr("GUILD_INFO", event));
		auto rs = co_await db::co_query("SELECT id, guilds.name AS guild_name, owner_id, (SELECT COUNT(*) FROM guild_members WHERE guild_id = guilds.id) AS member_count, game_users.* FROM guilds LEFT JOIN game_users ON guilds.owner_id = game_users.user_id WHERE guilds.name = ?", { guild_name });
		if (!rs.empty()) {
			std::string description{"# " + dpp::utility::markdown_escape(rs[0].at("guild_name")) + "\n\n" + tr("MEMBER_COUNT", event) + " " + rs[0].at("member_count") + "\n" };
			if (!rs[0].at("name").empty())
			description += "\n" + tr("OWNER", event) + " **" + dpp::utility::markdown_escape(rs[0].at("name")) + "**";
			embed.set_description(description);
		} else {
			embed.set_description(tr("NO_SUCH_GUILD", event));
		}
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	} else if (subcommand.name == "leave") {
		embed.set_title(tr("LEAVE_GUILD", event));
		auto g = co_await db::co_query("SELECT * FROM guild_members JOIN guilds ON guild_id = guilds.id WHERE user_id = ?", { event.command.usr.id });
		if (g.empty()) {
			embed.set_description(tr("NOT_A_GUILD_MEMBER", event));
		} else {
			co_await db::co_query("DELETE FROM guild_members WHERE user_id = ?", { event.command.usr.id });
			embed.set_description(tr("LEFT_GUILD", event) + " " + dpp::utility::markdown_escape(g[0].at("name")));
		}
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	} else if (subcommand.name == "rename") {
		auto param = subcommand.options[0].value;
		std::string guild_name = std::get<std::string>(param);
		embed.set_title(tr("RENAME_GUILD", event));
		auto g = co_await db::co_query("SELECT * FROM guild_members JOIN guilds ON guild_id = guilds.id WHERE user_id = ?", { event.command.usr.id });
		if (g.empty()) {
			embed.set_description(tr("NOT_A_GUILD_MEMBER", event));
			co_return;
		} else {
			if (g[0].at("owner_id") != event.command.usr.id.str()) {
				embed.set_description(tr("NOT_THE_GUILD_OWNER", event));
			} else {
				neutrino swear_check(event.owner, config::get("neutrino_user"), config::get("neutrino_password"));
				swear_filter_t swear_filter = co_await swear_check.co_contains_bad_word(guild_name);
				if (!swear_filter.clean) {
					bot.log(dpp::ll_warning, "Potty-mouth guild name " + guild_name + " censored for id " + event.command.usr.id.str());
					guild_name = swear_filter.censored_content;
				}
				auto exists = co_await db::co_query("SELECT id FROM guilds WHERE name = ?", {guild_name});
				if (!exists.empty()) {
					embed.set_description(tr("GUILD_EXISTS", event));
				} else {
					co_await db::co_query("UPDATE guilds SET name = ? WHERE id = ?", {g[0].at("guild_id")});
					embed.set_description(tr("RENAMED_GUILD_TO", event, dpp::utility::markdown_escape(g[0].at("name"))));
				}
			}
		}
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	}
	co_return;
}
