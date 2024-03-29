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
#include <ssod/commands/help.h>
#include <ssod/game_date.h>

dpp::slashcommand help_command::register_command(dpp::cluster& bot)
{
	return dpp::slashcommand("help", "Seven Spells Of Destruction Bot help", bot.me.id).set_dm_permission(true);
}

void help_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("Help - The Seven Spells Of Destruction")
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description("Welcome to **The Seven Spells Of Destruction**, an open world multi player role playing game played through Discord! There are not many commands for this bot as using it is simple. to get started, use `/start`. You can find a list of all the commands for the bot below:")
		.add_field("/help", "Bring about world peace, or just show this command\nShows information about the bot's commands", false)
		.add_field("/start", "Start a new character, or continue your game\nThe game is played via button clicks and interacting with bot messages.", false)
		.add_field("/lore", "Show lore and information about the game world, its locations and people\nUse button clicks to navigate the lore system.", false)
		.add_field("/map", "Show a map of the game world.\nThe bot will not show you where you are. It is up to you to keep track of your bearings!", false)
		.add_field("/vote", "Show how to vote for the bot on top.gg for loot drops", false)
		.add_field("/profile", "Show a user's profile.\nYou must use the username of the user in the game, not their discord username.", false)
		.add_field("/guild", "Create or join a guild", false)
		.add_field("/gender", "Set your gender on your user profile.\nThis is only used to set the gender of your profile picture (which is selected to match your character's profession and race). Please note only `male` or `female` are supported, due to image creation limitations.", false)
		.add_field("/premium", "Show a link to manage your premium, if you are a premium member.", false)
		.add_field("/bio", "Change your biography or profile picture; premium command.", false)
		.add_field("/rename", "Rename a weapon or armour piece; premium command.", false)
		.add_field("/info", "Show technical information about the bot.\nUnless you are having problems, you probably don't need to use this command.", false)
		;

	event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
}
