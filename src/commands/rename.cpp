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
#include <ssod/commands/rename.h>
#include <ssod/game_player.h>
#include <ssod/game_util.h>
#include <ssod/wildcard.h>
#include <fmt/format.h>

static void autocomplete(dpp::cluster& bot, const dpp::autocomplete_t& event, const std::string& uservalue) {
	if (!player_is_live(event)) {
		return;
	}
	player p = get_live_player(event, false);
	dpp::interaction_response ir(dpp::ir_autocomplete_reply);
	int count = 0;
	for (const item& i : p.possessions) {
		if (i.name.starts_with(uservalue) && i.flags.length() >= 2 && (i.flags[0] == 'W' || i.flags[0] == 'A') && isdigit(i.flags[1])) {
			ir.add_autocomplete_choice(dpp::command_option_choice(i.name, i.name));
			if (count++ > 24) {
				break;
			}
		}
	}
	bot.interaction_response_create(event.command.id, event.command.token, ir);
}

dpp::slashcommand rename_command::register_command(dpp::cluster& bot) {
	bot.on_autocomplete([&bot](const dpp::autocomplete_t & event) {
		if (event.name != "rename") {
			return;
		}
		std::string partial = std::get<std::string>(event.options[0].value);
		autocomplete(bot, event, partial);
	});

	return _(dpp::slashcommand("cmd_rename", "rename_desc", bot.me.id)
		.set_dm_permission(true)
		.add_option(dpp::command_option(dpp::co_string, "opt_item", "rename_item_desc", true).set_auto_complete(true))
		.add_option(dpp::command_option(dpp::co_string, "opt_name", "rename_name_desc", true).set_max_value(20)));
}

void rename_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.from->creator;
	auto rs = db::query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { event.command.usr.id });
	if (event.command.entitlements.empty() && rs.empty()) {
		premium_required(event);
		return;
	}
	if (!player_is_live(event)) {
		event.reply(dpp::message(_("NOPROFILE", event)).set_flags(dpp::m_ephemeral));
		return;
	}

	std::string oldname = std::get<std::string>(event.get_parameter("item"));
	std::string newname = std::get<std::string>(event.get_parameter("name"));
	player p = get_live_player(event, false);
	newname = replace_string(newname, ";", "");

	dpp::embed embed;
	embed.set_url("https://ssod.org/")
		.set_title(_("RENAME", event))
		.set_footer(dpp::embed_footer{ 
			.text = _("REQUESTED_BY", event, event.command.usr.format_username()),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(_("RENAMED", event, oldname, newname));

	for (item& i : p.possessions) {
		if (dpp::lowercase(i.name) == dpp::lowercase(oldname) && i.flags.length() >= 2 && (i.flags[0] == 'W' || i.flags[0] == 'A') && isdigit(i.flags[1])) {
			i.name = newname;
			if (dpp::lowercase(p.weapon.name) == dpp::lowercase(oldname)) {
				p.weapon.name = newname;
			} else if (dpp::lowercase(p.armour.name) == dpp::lowercase(oldname)) {
				p.armour.name = newname;
			}
			p.inv_change = true;
		}
	}
	update_live_player(event, p);
	p.save(event.command.usr.id);

	event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

}
