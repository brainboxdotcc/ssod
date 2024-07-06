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

struct unsetglobal_tag : public tag {
	unsetglobal_tag() { register_tag<unsetglobal_tag>(); }
	static constexpr std::string_view tags[]{"<unsetglobal"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;
		p_text = dpp::lowercase(remove_last_char(p_text));
		db::query("DELETE FROM game_global_flags WHERE flag = ?", {p_text});
		co_return;
	}
};

static unsetglobal_tag self_init;
