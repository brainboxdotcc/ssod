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

struct modifier_t {
	std::string name;
	long* score{nullptr};
	long max{9223372036854775807};
};

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

		const std::map<std::string, modifier_t> modifier_list = {
			{"stm", {"stamina", &current_player.stamina, current_player.max_stamina()}},
			{"skl", {"skill", &current_player.skill, current_player.max_skill()}},
			{"luck", {"luck", &current_player.luck, current_player.max_luck()}},
			{"lck", {"luck", &current_player.luck, current_player.max_luck()}},
			{"exp", {"experience", &current_player.experience, 9223372036854775807}},
			{"arm", {"armour", &current_player.armour.rating, 9223372036854775807}},
			{"wpn", {"weapon", &current_player.weapon.rating, 9223372036854775807}},
			{"spd", {"speed", &current_player.speed, current_player.max_speed()}},
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
			output << " ***" << (modifier < 1 ? "Subtract " : "Add ") << abs(modifier) << " " << m->second.name << "*** ";
			long old_value = current_player.get_level();
			*(m->second.score) += modifier;
			*(m->second.score) = std::min(*(m->second.score), m->second.max);
			long new_value = current_player.get_level();
			if (new_value > old_value && new_value > 1) {
				current_player.add_toast("# Level Up!\n\n## You are now level " + std::to_string(new_value) + "\nYou gain +1 to maximum stamina, skill, speed, sneak, and luck.");
			}
			p.words++;
		}
	}
};

static mod_tag self_init;
