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
#include <ssod/parser.h>

struct mod_tag : public tag {
	mod_tag() { register_tag<mod_tag>(); }
	static constexpr std::string_view tags[]{"<mod"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string mod;
		paragraph_content >> p_text;
		paragraph_content >> mod;
		mod = remove_last_char(mod); // remove '>'
		long modifier = atol(mod);
		std::string flag = "MOD" + p_text + mod;

		const std::map<std::string, std::pair<std::string, long*>> modifier_list = {
			{"stm", {"stamina", &current_player.stamina}},
			{"skl", {"skill", &current_player.skill}},
			{"luck", {"luck", &current_player.luck}},
			{"exp", {"experience", &current_player.experience}},
			{"arm", {"armour", &current_player.armour.rating}},
			{"wpn", {"weapon", &current_player.weapon.rating}},
			{"spd", {"speed", &current_player.speed}},
		};
		auto m = modifier_list.find(p_text);
		if (m != modifier_list.end()) {
			// No output if the player's been here before
			if (!current_player.has_flag(flag, p.id)) {
				current_player.add_flag(flag, p.id);
			} else {
				output << "***Make no changes to your " << m->first << "*** ";
				return;
			}
			output << " ***" << (modifier < 1 ? "Subtract " : "Add ") << abs(modifier) << " " << m->second.first << "*** ";
			*(m->second.second) += modifier;
			p.words++;
		}
	}
};

static mod_tag self_init;
