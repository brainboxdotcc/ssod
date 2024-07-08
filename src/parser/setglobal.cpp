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

struct setglobal_tag : public tag {
	setglobal_tag() { register_tag<setglobal_tag>(); }
	static constexpr std::string_view tags[]{"<setglobal"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;
		p_text = dpp::lowercase(remove_last_char(p_text));
		co_await db::co_query("INSERT INTO game_global_flags (flag) VALUES(?) ON DUPLICATE KEY UPDATE flag = ?", {p_text, p_text});
		co_await achievement_check("GLOBAL_STATE", current_player.event, current_player, {{"flag", p_text}});
		co_return;
	}
};

static setglobal_tag self_init;
