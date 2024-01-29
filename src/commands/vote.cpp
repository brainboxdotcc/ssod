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
#include <ssod/commands/vote.h>
#include <ssod/database.h>

dpp::slashcommand vote_command::register_command(dpp::cluster& bot)
{
	return dpp::slashcommand("vote", "Show how to vote for the bot to gain loot drops", bot.me.id);
}

void vote_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("Vote to gain loot drops")
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_description("[Vote for Seven Spells on top.gg](src/commands/map.cpp) to gain additional loot such as helpful potions every 12 hours!\n\nIf you do not already have a stamina or skill potion, and you vote for the bot, then your supply of that item will be replenished.\n\n**Use the potions repeatedly to max out your stats.**");
	event.reply(dpp::message()
		.add_embed(embed)
		.set_flags(dpp::m_ephemeral)
	);
}
