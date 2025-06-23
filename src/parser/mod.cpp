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
	static dpp::task<void> route(paragraph& p, std::string& lhs, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string mod;
		bool repeatable{false};

		paragraph_content >> lhs;

		if (lhs == "repeatable") {
			paragraph_content >> lhs;
			repeatable = true;
		}

		std::map<std::string, modifier_t> modifier_list = {
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

		for (uint8_t reg_no = 0; reg_no < 32; ++reg_no) {
			modifier_list.emplace("reg" + std::to_string(reg_no), modifier_t{"REGISTER", &current_player.regs[reg_no], 9223372036854775807});
		}

		paragraph_content >> mod;
		bool assignment = false;
		bool silent = false;
		long modifier = 0;
		mod = dpp::lowercase(mod);

		// Flag assignment form: <mod stat flag flagname>
		if (mod == "flag") {
			std::string flagname;
			paragraph_content >> flagname;
			flagname = remove_last_char(flagname);

			auto m = modifier_list.find(lhs);
			if (m != modifier_list.end()) {
				std::string val = current_player.get_flag(flagname);
				bool is_numeric = !val.empty() && std::all_of(val.begin(), val.end(), ::isdigit);

				if (is_numeric) {
					*(m->second.score) = atol(val);
					*(m->second.score) = std::min(*(m->second.score), m->second.max);
					*(m->second.score) = std::max(*(m->second.score), 0L);
					co_return;
				}
				current_player.event.owner->log(dpp::ll_debug, "MOD: non-numeric or missing flag '" + flagname + "' in location " + std::to_string(p.id));
			} else {
				current_player.event.owner->log(dpp::ll_warning, "Invalid MOD target stat '" + lhs + "' in location " + std::to_string(p.id));
			}
			co_return;
		}

		auto rhs = modifier_list.find(remove_last_char(mod));
		if (rhs != modifier_list.end()) {
			assignment = true;
		} else {
			modifier = atol(mod);
		}

		std::string flag = "MOD" + lhs + std::to_string(modifier);

		if (lhs.starts_with("reg") || mod.starts_with("reg")) {
			repeatable = true;
			silent = true;
		}

		auto m = modifier_list.find(lhs);
		if (m != modifier_list.end()) {
			if (assignment) {
				*(m->second.score) = *(rhs->second.score);
				*(m->second.score) = std::min(*(m->second.score), m->second.max);
				*(m->second.score) = std::max(*(m->second.score), 0L);
				co_return;
			}

			if (!repeatable) {
				if (!current_player.has_flag(flag, p.id)) {
					current_player.add_flag(flag, p.id);
				} else {
					output << " ***" << tr("MODNO", current_player.event, m->first) << "*** ";
					co_return;
				}
			}
			if (!silent) {
				output << " ***" << tr(modifier < 1 ? "MODNEG" : "MODPLUS", current_player.event, abs(modifier), m->second.name) << "*** ";
			}
			long old_value = current_player.get_level();
			*(m->second.score) += modifier;
			*(m->second.score) = std::min(*(m->second.score), m->second.max);
			*(m->second.score) = std::max(*(m->second.score), 0L);
			if (!silent) {
				co_await achievement_check("MOD", current_player.event, current_player, {{"modifier", modifier}, {"stat", lhs}});
			}
			long new_value = current_player.get_level();
			if (new_value > old_value && new_value > 1) {
				current_player.add_toast({ .message = tr("LEVELUP", current_player.event, new_value), .image = "level-up.png" });
				co_await achievement_check("LEVEL_UP", current_player.event, current_player, {{"level", new_value}});
			}
			p.words++;
			if (modifier < 0 || lhs == "notoriety" || lhs == "gold" || lhs == "silver") {
				p.safe = false;
			}
		} else {
			current_player.event.owner->log(dpp::ll_warning, "Invalid MOD " + lhs + " in location " + std::to_string(p.id));
		}
		co_return;
	}
};

static mod_tag self_init;
