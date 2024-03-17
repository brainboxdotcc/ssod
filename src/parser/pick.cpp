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

struct pick_tag : public tag {
	pick_tag() { register_tag<pick_tag>(); }
	static constexpr std::string_view tags[]{"<pick"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// pick up free items (one-choice)
		size_t max = current_player.max_inventory_slots();
		paragraph_content >> p_text;
		extract_to_quote(p_text, paragraph_content, '"');
		std::string ItemName = extract_value(p_text);
		paragraph_content >> p_text;
		std::string ItemVal = extract_value(p_text);
		if (!current_player.has_flag("PICKED", p.id)) {
			output << "\n **" << ItemName << "** ";
			output << directions[++p.links] << "\n";
			if (current_player.possessions.size() < max - 1) {
				p.navigation_links.push_back(
					nav_link{.paragraph = p.id, .type = nav_type_pick_one, .cost = 0, .monster = {}, .buyable = {.name = ItemName, .flags = ItemVal}, .prompt = "", .answer = "", .label = ""});
			} else {
				p.navigation_links.push_back(
					nav_link{.paragraph = p.id, .type = nav_type_disabled_link, .cost = 0, .monster = {}, .buyable = {}, .prompt = "", .answer = "", .label = "Inventory Full"});
			}
		} else {
			output << "\n **" << ItemName << "**\n";
		}
		p.words++;
	}
};

static pick_tag self_init;
