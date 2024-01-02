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
#include <ssod/commands/profile.h>
#include <ssod/database.h>
#include <ssod/game_player.h>
#include <ssod/emojis.h>

dpp::slashcommand profile_command::register_command(dpp::cluster& bot) {
	return dpp::slashcommand("profile", "View User Profile", bot.me.id)
		.add_option(dpp::command_option(dpp::co_string, "user", "User to view profile of", false));
}

void profile_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.from->creator;
	auto param = event.get_parameter("user");
	std::string user;
	player p;
	if (param.index() == 0) {
		p = get_live_player(event);
		user = p.name;
	} else {
		user = std::get<std::string>(event.get_parameter("user"));
	}
	auto rs = db::query("SELECT * FROM game_users WHERE name = ?", {user});
	if (rs.empty()) {
		event.reply("No such user. Remember, you must use an in-game nickname, not a discord username!");
	}
	p.experience = atol(rs[0].at("experience"));

	std::string content{"### Level " + std::to_string(p.get_level()) + "\n"};
	int percent = p.get_percent_of_current_level();
	for (int x = 0; x < 100; x += 10) {
		if (x < percent) {
			content += sprite::bar_green.get_mention();
		} else {
			content += sprite::bar_red.get_mention();
		}
	}
	content += " (" + std::to_string(percent) + "%)";

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("Profile: " + user)
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_description(content)
		.add_field("Stamina", sprite::health_heart.get_mention() + " " + rs[0].at("stamina"), true)
		.add_field("Skill", sprite::book07.get_mention() + " " + rs[0].at("skill"), true)
		.add_field("Luck", sprite::clover.get_mention() + " " + rs[0].at("luck"), true)
		.add_field("XP", sprite::medal01.get_mention() + " " + rs[0].at("experience"), true)
		.add_field("Speed", sprite::shoes03.get_mention() + " " + rs[0].at("speed"), true)
		.add_field("Sneak", sprite::throw05.get_mention() + " " + rs[0].at("sneak"), true)
		.add_field("Gold", sprite::gold_coin.get_mention() + " " + rs[0].at("gold"), true)
		.add_field("Armour", sprite::helm03.get_mention() + " " + rs[0].at("armour_rating"), true)
		.add_field("Weapon", sprite::axe013.get_mention() + " " + rs[0].at("weapon_rating"), true)
		;

	event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

}
