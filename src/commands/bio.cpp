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
#include <ssod/commands/bio.h>
#include <ssod/game_util.h>
#include <ssod/database.h>
#include <ssod/aes.h>

dpp::slashcommand bio_command::register_command(dpp::cluster& bot) {
	return dpp::slashcommand("bio", "Update player biography", bot.me.id)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "picture", "Set a custom profile picture")
			.add_option(dpp::command_option(dpp::co_attachment, "image", "Image to upload", true))
		)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "text", "Set custom biography")
			.add_option(dpp::command_option(dpp::co_string, "text", "Biography to set", true))
		);
}

void bio_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.from->creator;

	dpp::command_interaction cmd_data = event.command.get_command_interaction();
	auto subcommand = cmd_data.options[0];

	auto rs = db::query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { event.command.usr.id });
	if (rs.empty()) {
		event.reply(
			dpp::message("## Premium Required\n\nYou need [Seven Spells Premium](https://premium.ssod.org) to use this feature! Subscriptions are just £3 a month and give access to additional areas, automatic loot drops, and more!")
			.set_flags(dpp::m_ephemeral)
			.add_component(
				dpp::component().add_component(
					dpp::component()
					.set_type(dpp::cot_button)
					.set_id(security::encrypt("player_premium_cta"))
					.set_label("Get Premium")
					.set_url("https://premium.ssod.org")
					.set_style(dpp::cos_link)
				)
			)
		);
	} else  if (subcommand.name == "text") {
		auto param = subcommand.options[0].value;
		std::string text = std::get<std::string>(param);
		db::query("INSERT INTO character_bio (user_id, bio) VALUES(?, ?) ON DUPLICATE KEY UPDATE bio = ?", { event.command.usr.id, text, text });
		event.reply(dpp::message("Set bio " + text).set_flags(dpp::m_ephemeral));
	} else if (subcommand.name == "picture") {
		auto param = subcommand.options[0].value;
            	dpp::snowflake file_id = std::get<dpp::snowflake>(param);
		dpp::attachment att = event.command.get_resolved_attachment(file_id);		
		db::query("INSERT INTO character_bio (user_id, image_name) VALUES(?, ?) ON DUPLICATE KEY UPDATE image_name = ?", { event.command.usr.id, att.filename, att.filename });
		event.reply(dpp::message("Set picture " + att.filename).set_flags(dpp::m_ephemeral));
	}

}
