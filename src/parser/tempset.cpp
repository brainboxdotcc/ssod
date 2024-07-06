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
#include <ssod/parser.h>
#include <ssod/database.h>
#include <ssod/achievement.h>

struct tempset_tag : public tag {
	tempset_tag() { register_tag<tempset_tag>(); }
	static constexpr std::string_view tags[]{"<tempset"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// set a state-flag
		long lifetime{0};
		paragraph_content >> lifetime;
		paragraph_content >> p_text;
		p_text = dpp::lowercase(remove_last_char(p_text));
		long expiry = time(nullptr) + lifetime;
		db::query("INSERT INTO timed_flags (user_id, flag, expiry) VALUES(?,?,?) ON DUPLICATE KEY UPDATE expiry = ?", {
			current_player.event.command.usr.id, p_text, expiry, expiry
		});
		achievement_check("TEMP_STATE", current_player.event, current_player, {{"flag", p_text}});
		co_return;
	}
};

static tempset_tag self_init;
