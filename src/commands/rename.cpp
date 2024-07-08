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
#include <ssod/database.h>
#include <ssod/config.h>
#include <ssod/neutrino_api.h>

using namespace i18n;

static dpp::task<void> autocomplete(dpp::cluster& bot, const dpp::autocomplete_t& event, const std::string& uservalue) {
	if (!(co_await player_is_live(event))) {
		co_return;
	}
	player p = get_live_player(event, false);
	dpp::interaction_response ir(dpp::ir_autocomplete_reply);
	int count = 0;
	for (const stacked_item& i : p.possessions) {
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
	bot.on_autocomplete([&bot](const dpp::autocomplete_t & event) -> dpp::task<void> {
		if (event.name != "rename") {
			co_return;
		}
		std::string partial = std::get<std::string>(event.options[0].value);
		co_await autocomplete(bot, event, partial);
	});

	return tr(dpp::slashcommand("cmd_rename", "rename_desc", bot.me.id)
		.set_dm_permission(true)
		.add_option(dpp::command_option(dpp::co_string, "opt_item", "rename_item_desc", true).set_auto_complete(true))
		.add_option(dpp::command_option(dpp::co_string, "opt_name", "rename_name_desc", true).set_max_value(20)));
}

dpp::task<void> rename_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.from->creator;
	auto rs = co_await db::co_query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { event.command.usr.id });
	if (event.command.entitlements.empty() && rs.empty()) {
		premium_required(event);
		co_return;
	}
	if (!(co_await player_is_live(event))) {
		event.reply(dpp::message(tr("NOPROFILE", event)).set_flags(dpp::m_ephemeral));
		co_return;
	}

	std::string oldname = std::get<std::string>(event.get_parameter("item"));
	std::string newname = std::get<std::string>(event.get_parameter("name"));
	player p = get_live_player(event, false);
	newname = replace_string(newname, ";", "");

	auto r = co_await db::co_query("SELECT * FROM game_item_descs WHERE (name = ? OR name = ?) AND (quest_item = 1 OR sellable = 0)", {oldname, newname});
	if (!r.empty()) {
		event.reply(dpp::message(tr("INVALIDRENAME", event, oldname, newname)).set_flags(dpp::m_ephemeral));
		co_return;
	}
	neutrino swear_check(event.from->creator, config::get("neutrino_user"), config::get("neutrino_password"));
	swear_check.contains_bad_word(newname, [player_v = p, &bot, oldname, nn_v = newname, event](const swear_filter_t& swear_filter) {
		player p{player_v};
		std::string newname{nn_v};
		if (!swear_filter.clean) {
			newname = swear_filter.censored_content;
			bot.log(dpp::ll_warning, "Potty-mouth item name: " + nn_v + " censored for id: " + event.command.usr.id.str());
		}
		dpp::embed embed;
		embed.set_url("https://ssod.org/")
			.set_title(tr("RENAME", event))
			.set_footer(dpp::embed_footer{
				.text = tr("REQUESTED_BY", event, event.command.usr.format_username()),
				.icon_url = bot.me.get_avatar_url(),
				.proxy_url = "",
			})
			.set_colour(EMBED_COLOUR)
			.set_description(tr("RENAMED", event, oldname, newname));

		for (stacked_item& i : p.possessions) {
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
		return;
	});
	co_return;
}
