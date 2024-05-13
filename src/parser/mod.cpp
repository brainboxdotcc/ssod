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
		bool repeatable{false};
		paragraph_content >> p_text;
		if (p_text == "repeatable") {
			paragraph_content >> p_text;
			repeatable = true;
		}
		paragraph_content >> mod;
		mod = remove_last_char(mod); // remove '>'
		long modifier = atol(mod);
		std::string flag = "MOD" + p_text + mod;

		const std::map<std::string, modifier_t> modifier_list = {
			{"stm", {_("STAMINA", current_player.event), &current_player.stamina, current_player.max_stamina()}},
			{"skl", {_("SKILL", current_player.event), &current_player.skill, current_player.max_skill()}},
			{"luck", {_("LUCK", current_player.event), &current_player.luck, current_player.max_luck()}},
			{"lck", {_("LUCK", current_player.event), &current_player.luck, current_player.max_luck()}},
			{"exp", {"XP", &current_player.experience, 9223372036854775807}},
			{"arm", {_("ARMOUR", current_player.event), &current_player.armour.rating, 9223372036854775807}},
			{"wpn", {_("WEAPON", current_player.event), &current_player.weapon.rating, 9223372036854775807}},
			{"spd", {_("SPEED", current_player.event), &current_player.speed, current_player.max_speed()}},
			{"mana", {_("MANA", current_player.event), &current_player.mana, current_player.max_mana()}},
			{"rations", {_("RATIONS", current_player.event), &current_player.rations, 9223372036854775807}},
			{"notoriety", {_("NOTORIETY", current_player.event), &current_player.notoriety, 9223372036854775807}},
		};
		auto m = modifier_list.find(p_text);
		if (m != modifier_list.end()) {
			// No output if the player's been here before
			if (!repeatable) {
				if (!current_player.has_flag(flag, p.id)) {
					current_player.add_flag(flag, p.id);
				} else {
					output << " ***" << _("MODNO", current_player.event, m->first) << "*** ";
					return;
				}
			}
			output << " ***" << _(modifier < 1 ? "MODNEG" : "MODPLUS", current_player.event, abs(modifier), m->second.name) << "*** ";
			long old_value = current_player.get_level();
			*(m->second.score) += modifier;
			*(m->second.score) = std::min(*(m->second.score), m->second.max);
			*(m->second.score) = std::max(*(m->second.score), 0L);
			long new_value = current_player.get_level();
			if (new_value > old_value && new_value > 1) {
				current_player.add_toast(_("LEVELUP,", current_player.event, new_value));
			}
			p.words++;
			if (modifier < 0 || p_text == "notoriety") {
				/* Locations that remove stats are not safe locations */
				p.safe = false;
			}
		}
	}
};

static mod_tag self_init;
