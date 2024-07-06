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
#include <ssod/achievement.h>

using namespace i18n;

struct modifier_t {
	std::string name;
	long* score{nullptr};
	long max{9223372036854775807};
};

struct mod_tag : public tag {
	mod_tag() { register_tag<mod_tag>(); }
	static constexpr std::string_view tags[]{"<mod"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string mod;
		bool repeatable{false};
		paragraph_content >> p_text;
		if (p_text == "repeatable") {
			paragraph_content >> p_text;
			repeatable = true;
		}
		paragraph_content >> mod;
		long modifier = atol(mod);
		std::string flag = "MOD" + p_text + std::to_string(modifier);

		const std::map<std::string, modifier_t> modifier_list = {
			{"stm", {tr("STAMINA", current_player.event), &current_player.stamina, current_player.max_stamina()}},
			{"skl", {tr("SKILL", current_player.event), &current_player.skill, current_player.max_skill()}},
			{"luck", {tr("LUCK", current_player.event), &current_player.luck, current_player.max_luck()}},
			{"lck", {tr("LUCK", current_player.event), &current_player.luck, current_player.max_luck()}},
			{"exp", {"XP", &current_player.experience, 9223372036854775807}},
			{"arm", {tr("ARMOUR", current_player.event), &current_player.armour.rating, 9223372036854775807}},
			{"wpn", {tr("WEAPON", current_player.event), &current_player.weapon.rating, 9223372036854775807}},
			{"spd", {tr("SPEED", current_player.event), &current_player.speed, current_player.max_speed()}},
			{"mana", {tr("MANA", current_player.event), &current_player.mana, current_player.max_mana()}},
			{"rations", {tr("RATIONS", current_player.event), &current_player.rations, current_player.max_rations()}},
			{"notoriety", {tr("NOTORIETY", current_player.event), &current_player.notoriety, 9223372036854775807}},
			{"gold", {tr("GOLD", current_player.event), &current_player.gold, current_player.max_gold()}},
			{"silver", {tr("SILVER", current_player.event), &current_player.silver, current_player.max_silver()}},
		};
		auto m = modifier_list.find(p_text);
		if (m != modifier_list.end()) {
			// No output if the player's been here before
			if (!repeatable) {
				if (!current_player.has_flag(flag, p.id)) {
					current_player.add_flag(flag, p.id);
				} else {
					output << " ***" << tr("MODNO", current_player.event, m->first) << "*** ";
					co_return;
				}
			}
			output << " ***" << tr(modifier < 1 ? "MODNEG" : "MODPLUS", current_player.event, abs(modifier), m->second.name) << "*** ";
			long old_value = current_player.get_level();
			*(m->second.score) += modifier;
			*(m->second.score) = std::min(*(m->second.score), m->second.max);
			*(m->second.score) = std::max(*(m->second.score), 0L);
			achievement_check("MOD", current_player.event, current_player, {{"modifier", modifier}, {"stat", p_text}});
			long new_value = current_player.get_level();
			if (new_value > old_value && new_value > 1) {
				current_player.add_toast({ .message = tr("LEVELUP", current_player.event, new_value), .image = "level-up.png" });
				achievement_check("LEVEL_UP", current_player.event, current_player, {{"level", new_value}});
			}
			p.words++;
			if (modifier < 0 || p_text == "notoriety" || p_text == "gold" || p_text == "silver") {
				/* Locations that remove stats or give gold/silver are not safe locations */
				p.safe = false;
			}
		} else {
			std::cout << "Error: Invalid MOD " << p_text << " in location " << p.id << "\n";
		}
		co_return;
	}
};

static mod_tag self_init;
