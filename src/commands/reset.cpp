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
#include <ssod/commands/reset.h>
#include <ssod/database.h>
#include <ssod/aes.h>
#include <ssod/emojis.h>
#include <ssod/game_player.h>

dpp::slashcommand reset_command::register_command(dpp::cluster& bot)
{
	bot.on_button_click([&bot](const dpp::button_click_t& event) {
		if (!player_is_live(event)) {
			return;
		}
		player p = get_live_player(event);
		if (p.state != state_play || event.custom_id.empty()) {
			return;
		}
		std::string custom_id = security::decrypt(event.custom_id);
		if (custom_id.empty()) {
			return;
       		}
		if (custom_id == "player_reset") {
			dpp::cluster* bot = event.from->creator;
			dpp::embed embed = dpp::embed()
				.set_url("https://ssod.org/")
				.set_title("Player Reset Complete!")
				.set_footer(dpp::embed_footer{ 
					.text = "Requested by " + event.command.usr.format_username(), 	
					.icon_url = bot->me.get_avatar_url(), 
					.proxy_url = "",
				})
				.set_colour(0xd5b994)
				.set_description(
					"Your character has been cast into the void! All previous information has been deleted. You may now re-create a new character with `/start`.\n\nIf you have premium, your premium entitlement will continue on your new character."
				);
				
			event.reply(dpp::message()
				.add_embed(embed)
				.set_flags(dpp::m_ephemeral)
			);

			delete_live_player(event);
		}
	});
	return dpp::slashcommand("reset", "Completely reset character allowing for re-creation and re-roll", bot.me.id);
}

void reset_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("Seven Spells Of Destruction")
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 	
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_description(
			"Are you sure you want to reset? This will **__completely delete your character__** and is __**not reversible**__. You will then be able to `/start` to re-roll new stats and pick a new race and profession."
		);
		
	event.reply(dpp::message()
		.add_component(
			dpp::component().add_component(
				dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_reset"))
				.set_label("Yes, Reset!")
				.set_style(dpp::cos_danger)
				.set_emoji(sprite::skull.name, sprite::skull.id)
			)
		)
		.add_embed(embed)
		.set_flags(dpp::m_ephemeral)
	);
}
