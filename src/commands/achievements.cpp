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
#include <gen/emoji.h>
#include <fmt/format.h>
#include <ssod/aes.h>

using namespace i18n;

constexpr uint32_t page_size{6};

dpp::slashcommand achievements_command::register_command(dpp::cluster& bot) {
	return tr(dpp::slashcommand("cmd_achievements", "achievements_desc", bot.me.id)
		.set_interaction_contexts({dpp::itc_guild, dpp::itc_bot_dm, dpp::itc_private_channel})
		.add_option(dpp::command_option(dpp::co_string, "opt_user", "user_achievements_desc", false))
		.add_option(dpp::command_option(dpp::co_integer, "opt_page", "user_achievements_page_desc", false))
	);
}

dpp::task<void> achievements_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.owner;
	auto param = event.get_parameter("user");
	auto page_p = event.get_parameter("page");
	uint32_t page{1};
	std::string user;
	player p;
	bool self{false};
	if (param.index() == 0) {
		if (co_await player_is_live(event)) {
			p = get_live_player(event);
			user = p.name;
			self = true;
		}
	} else {
		user = std::get<std::string>(param);
	}
	if (page_p.index() != 0) {
		page = std::get<int64_t>(page_p);
	}
	auto rs = co_await db::co_query("SELECT * FROM game_users WHERE name = ?", {user});
	if (rs.empty()) {
		event.reply(dpp::message(tr(self ? "NOPROFILE" : "NOSUCHUSER", event)).set_flags(dpp::m_ephemeral));
		co_return;
	}

	std::stringstream content;

	content << "## " + tr("ACHIEVEMENTS", event) +  "\n\n";

	auto page_check = co_await db::co_query(
		"SELECT CEIL(COUNT(*) / ?) as page_count FROM `achievements_unlocked`"
		"JOIN achievements ON achievement_id = achievements.id AND achievements_unlocked.user_id = ? WHERE "
		"(achievements.secret = 0 OR (SELECT COUNT(*) FROM achievements_unlocked self WHERE self.achievement_id = achievements_unlocked.achievement_id AND self.user_id = ?) > 0) "
		"ORDER BY achievements_unlocked.created_at DESC",
		{ page_size, rs[0].at("user_id"), event.command.usr.id }
	);

	uint32_t max_pages = atoi(page_check.empty() ? "1" : page_check[0].at("page_count"));
	page = std::max(1U, std::min(page, max_pages));

	/**
	 * User can view all non-secret achievements, or secret achievements they have unlocked in common with the target user.
	 * Achievements are listed in reverse date order.
	 */
	auto ach = co_await db::co_query(
		"SELECT achievements.*, DATE_FORMAT(FROM_UNIXTIME(achievements_unlocked.created_at), '%D %b %Y, %H:%i') AS unlock_date FROM `achievements_unlocked`"
		"JOIN achievements ON achievement_id = achievements.id AND achievements_unlocked.user_id = ? WHERE "
		"(achievements.secret = 0 OR (SELECT COUNT(*) FROM achievements_unlocked self WHERE self.achievement_id = achievements_unlocked.achievement_id AND self.user_id = ?) > 0) "
		"ORDER BY achievements_unlocked.created_at DESC "
		"LIMIT ?,?",
		{ rs[0].at("user_id"), event.command.usr.id, (page - 1) * page_size, page_size }
	);

	size_t c{};
	for (const auto& achievement : ach) {
		std::string achievement_name{achievement.at("name")}, description{achievement.at("description")};
		if (event.command.locale.substr(0, 2) != "en") {
			auto translations = co_await db::co_query("SELECT * FROM translations WHERE row_id = ? AND language = ? AND table_col IN ('achievements/description', 'achievements/name') ORDER BY table_col", {achievement.at("id"), event.command.locale.substr(0, 2)});
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
		std::string trophy{sprite::bronzecoin.format()};
		if (xp > 10 && xp < 100) {
			trophy = sprite::silvercoin.format();
		} else if (xp >= 100) {
			trophy = sprite::goldcoin.format();
		}
		content << "<:" << trophy << "> __**" << achievement_name << "**__\n";
		content << "[" << description << "](https://images.ssod.org/resource/achievements/" << achievement.at("emoji") << ")\n";
		content << tr("UNLOCKED", event) << " " << achievement.at("unlock_date") << "\n\n";
		++c;

		if (content.str().length() > 5500) {
			content << "(and " << (ach.size() - c) << " more...)";
			break;
		}
	}

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title(tr("ACHIEVEMENTS_LIST", event) + ": " + dpp::utility::markdown_escape(user))
		.set_footer(dpp::embed_footer{ 
			.text = tr("ACH_FOOTER", event, dpp::utility::markdown_escape(user), page, max_pages),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_thumbnail("https://images.ssod.org/resource/achievement.png")
		.set_colour(EMBED_COLOUR)
		.set_description(content.str())
	;

	event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

	co_return;
}
