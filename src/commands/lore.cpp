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
#include <ssod/commands/lore.h>
#include <ssod/database.h>
#include <ssod/sentry.h>
#include <ssod/game_date.h>

dpp::slashcommand lore_command::register_command(dpp::cluster& bot)
{
	return dpp::slashcommand("lore", "Show lore pages about the game world", bot.me.id);
}

void lore_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("Encyclopaedia Cryptillius")
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0x7aff7a)
		.set_image("attachment://app_logo.png")
		.set_description("Select a choice from the options below to read information about the game world, its characters and your quest's background.");
	event.reply(dpp::message()
		.add_embed(embed)
		.set_flags(dpp::m_ephemeral)
		.add_file("map.jpg", dpp::utility::read_file("../resource/app_logo.png"))
	);
}
