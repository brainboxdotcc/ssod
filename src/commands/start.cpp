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
#include <dpp/dpp.h>
#include <fmt/format.h>
#include <ssod/database.h>
#include <ssod/commands/start.h>
#include <ssod/game_player.h>
#include <ssod/game.h>
#include <ssod/aes.h>
#include <ssod/emojis.h>
#include <ssod/config.h>

using namespace i18n;

dpp::slashcommand start_command::register_command(dpp::cluster& bot)
{
	bot.on_button_click([](const dpp::button_click_t &event) {
		if (player_is_live(event)) {
			return;
		}
		player p_old = get_registering_player(event);
		std::string custom_id = security::decrypt(event.custom_id);
		if (custom_id.empty()) {
			return;
		}
		dpp::cluster& bot = *(event.from->creator);
		bot.log(dpp::ll_debug, event.command.usr.id.str() + " button click: state: " + std::to_string(p_old.state) + " id: " + custom_id);
		if (custom_id == "player_reroll" && p_old.state == state_roll_stats) {
			event.reply();
			player p_new(true);
			p_new.event = p_old.event;
			update_registering_player(event, p_new);
			p_new.event.edit_original_response(p_new.get_registration_message(bot, event));
		} else if (custom_id == "player_herb_spell_selection" && p_old.state == state_roll_stats) {
			event.reply();
			p_old.state = state_pick_magic;
			p_old.stamina += bonuses_numeric(1, p_old.race, p_old.profession);
			p_old.skill += bonuses_numeric(2, p_old.race, p_old.profession);
			p_old.luck += bonuses_numeric(3, p_old.race, p_old.profession);
			p_old.sneak += bonuses_numeric(4, p_old.race, p_old.profession);
			p_old.speed += bonuses_numeric(5, p_old.race, p_old.profession);
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_magic_selection_message(bot, event));
		} else if (custom_id == "player_name" && p_old.state == state_pick_magic) {
			p_old.state = state_name_player;
			update_registering_player(event, p_old);
			dpp::interaction_modal_response modal(security::encrypt("name_character"), tr("NAME", event), {
				dpp::component()
				.set_label(tr("ENTERNAME", event))
				.set_id(security::encrypt("player_set_name"))
				.set_type(dpp::cot_text)
				.set_placeholder("Sir Discordia Of Chattingham")
				.set_min_length(1)
				.set_required(true)
				.set_max_length(64)
				.set_text_style(dpp::text_short)
			});
			event.dialog(modal);
		} else if (custom_id.substr(0, 4) == "lore") {
			/* Do nothing, this is handled by a different part of the bot */
		} else {
			event.reply(dpp::message(tr("EXPIRED", event, sprite::skull.get_mention())).set_flags(dpp::m_ephemeral));
		}
	});
	
	bot.on_select_click([](const dpp::select_click_t &event) {
		if (player_is_live(event)) {
			return;
		}
		event.reply();
		player p_old = get_registering_player(event);
		std::string custom_id = security::decrypt(event.custom_id);
		if (custom_id.empty()) {
			return;
		}
		dpp::cluster& bot = *(event.from->creator);
		if (custom_id == "select_player_race" && p_old.state == state_roll_stats && !event.values.empty()) {
			p_old.race = (player_race)atoi(event.values[0]);
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_registration_message(bot, event));
		} else if (custom_id == "select_player_gender" && p_old.state == state_roll_stats && !event.values.empty()) {
			p_old.gender = event.values[0];
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_registration_message(bot, event));
		} else if (custom_id == "select_player_profession" && p_old.state == state_roll_stats && !event.values.empty()) {
			p_old.profession = (player_profession)atoi(event.values[0]);
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_registration_message(bot, event));
		} else if (custom_id == "select_player_herbs" && p_old.state == state_pick_magic) {
			p_old.herbs.clear();
			for (const auto & h : event.values) {
				p_old.herbs.push_back(item{ .name = h, .flags = "HERB"});
			}
			p_old.inv_change = true;
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_magic_selection_message(bot, event));
		} else if (custom_id == "select_player_spells" && p_old.state == state_pick_magic) {
			p_old.spells.clear();
			for (const auto & s : event.values) {
				p_old.spells.push_back(item{ .name = s, .flags = "SPELL"});
			}
			p_old.inv_change = true;
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_magic_selection_message(bot, event));
		} else {
			event.reply(dpp::message(tr("EXPIRED", event, sprite::skull.get_mention())).set_flags(dpp::m_ephemeral));
		}
	});
	bot.on_form_submit([&bot](const dpp::form_submit_t & event) {
		if (player_is_live(event)) {
			return;
		}
		player p_old = get_registering_player(event);
		std::string custom_id = security::decrypt(event.custom_id);
		if (custom_id.empty()) {
			return;
		}
		if (custom_id == "name_character" && p_old.state == state_name_player) {
			std::string name = std::get<std::string>(event.components[0].components[0].value);
			auto check = db::query("SELECT * FROM game_users WHERE name = ?", {name});
			if (!check.empty()) {
				event.reply();
				dpp::message m = p_old.get_magic_selection_message(bot, event);
				m.embeds[0].description += "\n\n## " + tr("EXISTS", event);
				p_old.event.edit_original_response(m);
				return;
			}
			p_old.event.delete_original_response();
			p_old.name = name;
			p_old.state = state_play;
			update_registering_player(event, p_old);
			// Save to database and overwrite backup state
			p_old.save(event.command.usr.id, true);
			p_old.event = event;
			move_from_registering_to_live(event, p_old);
			db::query("DELETE FROM game_default_spells WHERE user_id = ?", {event.command.usr.id});
			db::query("INSERT INTO game_default_spells (user_id, name, flags) SELECT user_id, item_desc, item_flags FROM game_owned_items WHERE user_id = ? AND item_flags in ('HERB','SPELL')", {event.command.usr.id});
			continue_game(event, p_old);
			bot.log(dpp::ll_info, "New player creation: " + name + " for id: " + event.command.usr.id.str());
		}
	});
	return tr(dpp::slashcommand("cmd_start", "start_desc", bot.me.id).set_dm_permission(true));
}

void start_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;

	if (config::exists("dev") && config::get("dev") == true) {
		auto admin_rs = db::query("SELECT * FROM game_admins WHERE user_id = ?", {event.command.usr.id});
		if (admin_rs.empty()) {
			event.reply("This is the development copy of Seven Spells Of Destruction. Only developers of the game may play on this instance.");
			return;
		}
	}

	if (player_is_live(event)) {
		player p = get_live_player(event);
		send_chat(event.command.usr.id, p.paragraph, "", "join");
		continue_game(event, p);
		return;
	}

	player p = get_registering_player(event);
	p.state = state_roll_stats;
	p.event = event;
	update_registering_player(event, p);

	event.reply(p.get_registration_message(*bot, event));
}
