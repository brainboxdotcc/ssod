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
#include <ssod/commands/gender.h>
#include <ssod/database.h>
#include <ssod/game_player.h>

dpp::slashcommand gender_command::register_command(dpp::cluster& bot) {
	return dpp::slashcommand("gender", "Set player's in-game gender for their profile image (male or female)", bot.me.id)
		.set_dm_permission(true)
		.add_option(
			dpp::command_option(dpp::co_string, "gender", "Gender to set", true)
			.add_choice(dpp::command_option_choice("Male", "male"))
			.add_choice(dpp::command_option_choice("Female", "female"))
		);
}

void gender_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.from->creator;
	auto param = event.get_parameter("gender");
	std::string new_gender = std::get<std::string>(param);
	if (!player_is_live(event)) {
		event.reply(dpp::message("You do not have a profile yet. You must create a character by using the `/start` command!").set_flags(dpp::m_ephemeral));
		return;
	}
	db::query("UPDATE game_users SET gender = ? WHERE user_id = ?", {new_gender, event.command.usr.id});
	db::query("UPDATE game_default_users SET gender = ? WHERE user_id = ?", {new_gender, event.command.usr.id});

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("Set Gender")
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description("Player gender set to **" + new_gender + "**\n\nThis change is cosmetic only and used for display of the character image on your `/profile`.");
		;

	event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

}
