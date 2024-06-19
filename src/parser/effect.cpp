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
#include <ssod/game_util.h>

struct effect_tag : public tag {
	effect_tag() { register_tag<effect_tag>(); }
	static constexpr std::string_view tags[]{"<effect"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;
		std::string effect_name = remove_last_char(p_text);
		trigger_effect(*(current_player.event.from->creator), current_player.event, current_player, "Environmental", effect_name);
		p.words++;
	}
};

static effect_tag self_init;
