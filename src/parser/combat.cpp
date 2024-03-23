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

struct combat_tag : public tag {
	combat_tag() { register_tag<combat_tag>(); }
	static constexpr std::string_view tags[]{"<combat"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// combat tag
		p.links++;
		p.safe = false;
		std::string flag_bubonic = "[gamestate_bubonic_plague";
		std::string flag_blood = "[gamestate_blood_plague";
		if (current_player.gotfrom.find(flag_bubonic) != std::string::npos) {
			current_player.add_stamina(-4);
			current_player.add_toast("The bubonic plague takes its toll on your body, subtracting 4 stamina..."
						 "\n\nFind an antidote or healer before this proves fatal!");
		}
		if (current_player.gotfrom.find(flag_blood) != std::string::npos) {
			current_player.add_stamina(-3);
			current_player.add_toast("The blood plague takes its toll on your body, subtracting 3 stamina..."
						 "\n\nFind an antidote or healer before this proves fatal!");
		}
		paragraph_content >> p_text;
		extract_to_quote(p_text, paragraph_content, '"');
		std::string monster_name = extract_value(p_text);
		paragraph_content >> p_text;
		long monster_skill = extract_value_number(p_text);
		paragraph_content >> p_text;
		long monster_stamina = extract_value_number(p_text);
		paragraph_content >> p_text;
		long monster_armour = extract_value_number(p_text);
		paragraph_content >> p_text;
		long monster_weapon = extract_value_number(p_text);
		if (p.current_fragment == current_player.after_fragment) {
			// when combat link is finished it goes back to the
			// paragraph it came from, but the next fragment of it.
			// fragments can only be requested on a paragraph
			// that contains at least one combat.
			
			output << fmt::format(
				"\n```ansi\n⚔ \033[2;34m{0:16s}\033[0m \033[2;31mSTM\033[0m:\033[2;33m{1:2d}\033[0m \033[2;31mSKL\033[0m:\033[2;33m{2:2d}\033[0m \033[2;31mARM\033[0m:\033[2;33m{3:2d}\033[0m \033[2;31mWPN\033[0m:\033[2;33m{4:2d}\033[0m {5}\n```\n",
				monster_name.substr(0, 16),
				monster_stamina,
				monster_skill,
				monster_armour,
				monster_weapon,
				directions[p.links]
				);
			p.words++;
			p.navigation_links.push_back(nav_link{
				.paragraph = p.id,
				.type = nav_type_combat,
				.cost = 0,
				.monster = { 
					.name = monster_name,
					.stamina = monster_stamina,
					.skill = monster_skill,
					.armour = monster_armour,
					.weapon = monster_weapon,
				},
				.buyable = {},
				.prompt = "",
				.answer = "",
				.label = ""
			});
			throw parse_end_exception();
		}
		p.current_fragment++;
	}
};

static combat_tag self_init;
