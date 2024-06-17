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
#include <ssod/commands/achievements.h>
#include <ssod/game_player.h>
#include <ssod/emojis.h>
#include <fmt/format.h>

using namespace i18n;

dpp::slashcommand achievements_command::register_command(dpp::cluster& bot) {
	return tr(dpp::slashcommand("cmd_achievements", "achievements_desc", bot.me.id)
		.set_dm_permission(true)
		.add_option(dpp::command_option(dpp::co_string, "opt_user", "user_achievements_desc", false)));
}

void achievements_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.from->creator;
	auto param = event.get_parameter("user");
	std::string user;
	player p;
	bool self{false};
	if (param.index() == 0) {
		if (player_is_live(event)) {
			p = get_live_player(event);
			user = p.name;
			self = true;
		}
	} else {
		user = std::get<std::string>(param);
	}
	auto rs = db::query("SELECT * FROM game_users WHERE name = ?", {user});
	if (rs.empty()) {
		event.reply(dpp::message(tr(self ? "NOPROFILE" : "NOSUCHUSER", event)).set_flags(dpp::m_ephemeral));
		return;
	}

	std::stringstream content;

	content << "## " + tr("ACHIEVEMENTS", event) +  "\n\n";

	/**
	 * User can view all non-secret achievements, or secret achievements they have unlocked in common with the target user.
	 * Achievements are listed in reverse date order.
	 */
	auto ach = db::query(
		"SELECT achievements.*, DATE_FORMAT(FROM_UNIXTIME(achievements_unlocked.created_at), '%D %b %Y, %H:%i') AS unlock_date FROM `achievements_unlocked`"
		"JOIN achievements ON achievement_id = achievements.id AND achievements_unlocked.user_id = ? WHERE "
		"(achievements.secret = 0 OR (SELECT COUNT(*) FROM achievements_unlocked self WHERE self.achievement_id = achievements_unlocked.achievement_id AND self.user_id = ?) > 0) "
		"ORDER BY achievements_unlocked.created_at DESC",
		{ rs[0].at("user_id"), event.command.usr.id }
	);

	size_t c{};
	for (const auto& achievement : ach) {
		std::string achievement_name{achievement.at("name")}, description{achievement.at("description")};
		if (event.command.locale.substr(0, 2) != "en") {
			auto translations = db::query("SELECT * FROM translations WHERE row_id = ? AND language = ? AND table_col IN ('achievements/description', 'achievements/name') ORDER BY table_col", {achievement.at("id"), event.command.locale.substr(0, 2)});
			if (translations.size() == 2) {
				description = translations[0].at("translation");
				achievement_name = translations[1].at("translation");
			}
		}
		/**
		 * Bronze: 1-10 XP award achievement
		 * Silver: 11-99 XP award achievement
		 * Gold: 100+ XP award achievement
		 */
		long xp = atol(achievement.at("xp"));
		std::string trophy{sprite::bronze_coin.format()};
		if (xp > 10 && xp < 100) {
			trophy = sprite::silver_coin.format();
		} else if (xp >= 100) {
			trophy = sprite::gold_coin.format();
		}
		content << "<:" << trophy << "> __**" << achievement_name << "**__\n";
		content << "[" << description << "](https://images.ssod.org/resource/achievements/" << achievement.at("emoji") << ")\n";
		content << tr("UNLOCKED", event) << " " << achievement.at("unlock_date") << "\n\n";
		++c;

		if (content.str().length() > 5700) {
			content << "(and " << (ach.size() - c) << " more...)";
			break;
		}
	}

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title(tr("ACHIEVEMENTS_LIST", event) + ": " + dpp::utility::markdown_escape(user))
		.set_footer(dpp::embed_footer{ 
			.text = tr("REQUESTED_BY", event, event.command.usr.format_username()),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_thumbnail("https://images.ssod.org/resource/achievement.png")
		.set_colour(EMBED_COLOUR)
		.set_description(content.str())
	;

	event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));	
}
