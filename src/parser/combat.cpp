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
#include <ssod/game_player.h>
#include <fmt/format.h>
#include <ssod/database.h>
#include <ssod/achievement.h>

using namespace i18n;

struct combat_tag : public tag {
	combat_tag() { register_tag<combat_tag>(); }
	static constexpr std::string_view tags[]{"<combat"};

	static long calc_xp_worth(player&p, long stamina, long skill, long armour, long weapon) {
		long xp = std::max(skill - p.max_skill(), 1L);
		if (xp < 1) {
			xp = 1;
		}
		if (skill > p.skill * 1.25) {
			xp *= 1.5;
		}
		return xp;
	}

	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// combat tag
		p.links++;
		p.safe = false;
		bool leveled{};
		paragraph_content >> p_text;
		if (dpp::lowercase(p_text) == "leveled") {
			leveled = true;
			paragraph_content >> p_text;
		}
		extract_to_quote(p_text, paragraph_content, '"');
		std::string monster_name = extract_value(p_text);
		std::string english_monster_name{monster_name};
		paragraph_content >> p_text;
		long monster_skill = extract_value_number(p_text);
		if (leveled) {
			monster_skill += current_player.get_level() / 4;
		}
		paragraph_content >> p_text;
		long monster_stamina = extract_value_number(p_text);
		if (leveled) {
			monster_stamina += current_player.get_level();
		}
		paragraph_content >> p_text;
		long monster_armour = extract_value_number(p_text);
		if (leveled) {
			monster_armour = std::min(monster_armour + current_player.get_level() / 6L, 6L);
		}
		paragraph_content >> p_text;
		long monster_weapon = extract_value_number(p_text);
		if (leveled) {
			monster_weapon = std::min(monster_weapon + current_player.get_level() / 6L, 6L);
		}
		if (p.current_fragment == current_player.after_fragment) {
			// when combat link is finished it goes back to the
			// paragraph it came from, but the next fragment of it.
			// fragments can only be requested on a paragraph
			// that contains at least one combat.


			if (current_player.event.command.locale != "en") {
				auto translation = db::query("SELECT * FROM translations WHERE row_id = ? AND language = ? AND table_col = ?", {
					0, current_player.event.command.locale.substr(0, 2), monster_name
				});
				if (!translation.empty()) {
					monster_name = translation[0].at("translation");
				}
			}

			if (!p.sick) {
				/* Once per paragraph, check for existing illnesses and apply debuffs */
				auto illnesses = db::query("SELECT * FROM diseases");
				for (auto& illness : illnesses) {
					std::string flag = "gamestate_" + illness.at("flag") + "%";
					if (!db::query("SELECT kv_value FROM kv_store WHERE user_id = ? AND kv_key LIKE ?", {current_player.event.command.usr.id, flag}).empty()) {
						std::string name{illness.at("name")};
						if (current_player.event.command.locale != "en") {
							auto translation = db::query("SELECT * FROM translations WHERE row_id = ? AND language = ? AND table_col = ?", {
								illness.at("id"), current_player.event.command.locale.substr(0, 2), "diseases/name"
							});
							if (!translation.empty()) {
								name = translation[0].at("translation");
							}

						}
						current_player.add_stamina(-atol(illness.at("stamina_debuff").c_str()));
						current_player.add_toast({ .message = tr("DISEASED", current_player.event, name, illness.at("stamina_debuff")), .image = "diseased.png" });
						achievement_check("PLAGUE", current_player.event, current_player, {{"loss", atol(illness.at("stamina_debuff").c_str())}});
						p.sick = true;
					}
				}
			}

			output << fmt::format(
				"\n```ansi\n⚔ \033[2;34m{0:16s}\033[0m \033[2;31mSTM\033[0m:\033[2;33m{1:2d}\033[0m \033[2;31mSKL\033[0m:\033[2;33m{2:2d}\033[0m \033[2;31mARM\033[0m:\033[2;33m{3:2d}\033[0m \033[2;31mWPN\033[0m:\033[2;33m{4:2d}\033[0m {5}\n```\n",
				dpp::utility::utf8substr(monster_name, 0, leveled ? 14 : 16) + (leveled ? " ⭐" : ""),
				monster_stamina,
				monster_skill,
				monster_armour,
				monster_weapon,
				directions[p.links]
				);
			p.words++;
			if (current_player.stamina > 0) {
				p.navigation_links.push_back(nav_link{
					.paragraph = p.id,
					.type = nav_type_combat,
					.cost = 0,
					.monster = {
						.name = english_monster_name.substr(0, 40),
						.stamina = monster_stamina,
						.skill = monster_skill,
						.armour = monster_armour,
						.weapon = monster_weapon,
						.xp_value = calc_xp_worth(current_player, monster_stamina, monster_skill, monster_armour, monster_weapon),
					},
					.buyable = {},
					.prompt = "",
					.answer = "",
					.label = ""
				});
			}
			throw parse_end_exception();
		}
		p.current_fragment++;
		co_return;
	}
};

static combat_tag self_init;
