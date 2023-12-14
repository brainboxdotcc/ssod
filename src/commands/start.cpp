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
#include <dpp/dpp.h>
#include <fmt/format.h>
#include <ssod/database.h>
#include <ssod/commands/start.h>
#include <ssod/database.h>
#include <ssod/game_player.h>

dpp::slashcommand start_command::register_command(dpp::cluster& bot)
{
	bot.on_button_click([&bot](const dpp::button_click_t &event) {
		if (player_is_live(event.command.usr.id)) {
			return;
		}
		player p_old = get_registering_player(event);
		bot.log(dpp::ll_debug, "button click: state: " + std::to_string(p_old.state) + " id: " + event.custom_id);
		dpp::cluster& bot = *(event.from->creator);
		if (event.custom_id == "player_reroll" && p_old.state == state_roll_stats) {
			event.reply();
			player p_new(true);
			p_new.event = p_old.event;
			update_registering_player(event, p_new);
			p_new.event.edit_original_response(p_new.get_registration_message(bot, event));
		} else if (event.custom_id == "player_herb_spell_selection" && p_old.state == state_roll_stats) {
			event.reply();
			p_old.state = state_pick_magic;
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_magic_selection_message(bot, event));
		} else if (event.custom_id == "player_name" && p_old.state == state_pick_magic) {
			p_old.state = state_name_player;
			update_registering_player(event, p_old);
			dpp::interaction_modal_response modal("name_character", "Name Your Adventurer");
			modal.add_component(
				dpp::component()
				.set_label("Enter Your Character's Name")
				.set_id("player_set_name")
				.set_type(dpp::cot_text)
				.set_placeholder("Sir Discordia Of Chattingham")
				.set_min_length(1)
				.set_required(true)
				.set_max_length(64)
				.set_text_style(dpp::text_short)
			);
			event.dialog(modal);
		} else {
			event.reply("State error");
		}
	});
	bot.on_select_click([&bot](const dpp::select_click_t &event) {
		if (player_is_live(event.command.usr.id)) {
			return;
		}
		event.reply();
		player p_old = get_registering_player(event);
		dpp::cluster& bot = *(event.from->creator);
		if (event.custom_id == "select_player_race" && p_old.state == state_roll_stats) {
			p_old.race = (player_race)atoi(event.values[0].c_str());
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_registration_message(bot, event));
		} else if (event.custom_id == "select_player_profession" && p_old.state == state_roll_stats) {
			p_old.profession = (player_profession)atoi(event.values[0].c_str());
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_registration_message(bot, event));
		} else if (event.custom_id == "select_player_herbs" && p_old.state == state_pick_magic) {
			p_old.herbs.clear();
			for (const auto & h : event.values) {
				p_old.herbs.push_back(item{ .name = h, .flags = "HERB"});
			}
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_magic_selection_message(bot, event));
		} else  if (event.custom_id == "select_player_spells" && p_old.state == state_pick_magic) {
			p_old.spells.clear();
			for (const auto & s : event.values) {
				p_old.spells.push_back(item{ .name = s, .flags = "SPELL"});
			}
			update_registering_player(event, p_old);
			p_old.event.edit_original_response(p_old.get_magic_selection_message(bot, event));
		} else {
			event.reply("State error");
		}
	});
	return dpp::slashcommand("start", "Start a new character or resume game", bot.me.id);
}

void start_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;

	if (player_is_live(event.command.usr.id)) {
		event.reply("Player live - Not implemented");
		return;
	}

	player p = get_registering_player(event);
	p.state = state_roll_stats;
	p.event = event;
	update_registering_player(event, p);
	//p.save(event.command.usr.id, true);

	event.reply(p.get_registration_message(*bot, event));
}
