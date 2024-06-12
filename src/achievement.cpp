/************************************************************************************
 *
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023,2024 Craig Edwards <brain@ssod.org>
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
#include <dpp/dpp.h>
#include <string>
#include <ssod/ssod.h>
#include <ssod/game_player.h>
#include <ssod/paragraph.h>
#include <ssod/game_util.h>
#include <ssod/database.h>
#include <ssod/js.h>

using namespace i18n;

void achievement_check(const std::string& event_type, const dpp::interaction_create_t& event, player p, std::map<std::string, json> variables, const paragraph& para) {
	auto achievements = db::query("SELECT * FROM achievements WHERE enabled = 1 AND event_type = ? AND check_event IS NOT NULL", {event_type});
	for (const auto& achievement : achievements) {
		js::run(achievement.at("check_event"), (paragraph&)para, p, variables);
	}
}

void unlock_achievement(player& p, const db::row& achievement) {
	std::string name{achievement.at("name")}, description{achievement.at("description")};
	std::stringstream content;

	if (p.event.command.locale.substr(0, 2) != "en") {
		auto translations = db::query("SELECT * FROM translations WHERE row_id = ? AND language = ? AND table_col IN ('achievements/description', 'achievements/name') ORDER BY table_col", {achievement.at("id"), p.event.command.locale.substr(0, 2)});
		if (translations.size() == 2) {
			description = translations[0].at("translation");
			name = translations[1].at("translation");
		}
	}

	content << "__" << name << "__";
	content << "\n\n";
	content << description;
	content << "\n\n";
	content << tr("PLUS_TEN_XP", p.event);

	p.add_toast(toast{ .message = "## " + tr("ACH_UNLOCK", p.event) + "\n\n" + content.str(), .image = "" });
	p.add_experience(10);
}

