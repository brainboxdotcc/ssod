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
#include <ssod/achievement.h>

struct drop_tag : public tag {
	drop_tag() { register_tag<drop_tag>(); }
	static constexpr std::string_view tags[]{"<drop"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;
		std::string i{p_text};
		while (p_text.length() && *p_text.rbegin() != '>') {
			paragraph_content >> p_text;
			i += " " + p_text;
		}
		i = remove_last_char(i); // remove '>'
		current_player.drop_possession(item{ .name = i, .flags = "" });
		current_player.drop_spell(item{ .name = i, .flags = "" });
		current_player.drop_herb(item{ .name = i, .flags = "" });
		achievement_check("DISCARD", current_player.event, current_player, {{"name", i}});
	}
};

static drop_tag self_init;
