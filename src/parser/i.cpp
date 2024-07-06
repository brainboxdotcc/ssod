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
#include <ssod/game_util.h>

struct i_tag : public tag {
	i_tag() { register_tag<i_tag>(); }
	static constexpr std::string_view tags[]{"<i"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		p.trader = true;
		size_t max = current_player.max_inventory_slots();
		// purchase item tag
		paragraph_content >> p_text; // always: NAME="ItemName"
		std::string Value{"[none]"}, Cost;
		std::string ItemName = extract_value(p_text);
		paragraph_content >> p_text; // may be: VALUE="Flags" / COST="cost">
		while (p_text.find("=") == std::string::npos) {
			ItemName += " " + p_text;
			paragraph_content >> p_text;
			if (ItemName.length() && *ItemName.rbegin() == '"') {
				ItemName = remove_last_char(ItemName);
			}
		}

		if (p_text.length() && *p_text.rbegin() != '>') {
			// process VALUE token
			Value = extract_value(p_text);
			paragraph_content >> p_text; // read COST token that MUST now follow on
		}

		// process COST token here: COST="cost">
		Cost = extract_value(p_text);
		output << "\n**Buy: " << ItemName;
		bool special{false};
		if (current_player.has_possession(ItemName) || current_player.has_spell(ItemName) || current_player.has_herb(ItemName)) {
			output << " ✅ [In Inventory]";
		}
		if (dpp::lowercase(ItemName) == "horse" || dpp::lowercase(ItemName) == "pack pony" || dpp::lowercase(ItemName) == "donkey" || dpp::lowercase(ItemName) == "mule") {
			special = true;
			if (current_player.has_flag("horse")) {
				output << " 🐴 [You already have a mount]";
			}
		}
		if (dpp::lowercase(ItemName) == "backpack" || dpp::lowercase(ItemName) == "pack") {
			special = true;
			if (current_player.has_flag("pack")) {
				output << " ✅ [Owned]";
			}
		}
		if (dpp::lowercase(ItemName) == "saddle bags") {
			special = true;
			if (current_player.has_flag("saddlebags")) {
				output << " ✅ [Owned]";
			}
		}
		output << "** (*" << Cost << " gold*) - " << describe_item(Value, ItemName, current_player.event) << "\n";
		p.words++;

		p.links++;
		output << directions[p.links] << "\n";
		if (special) {
			p.navigation_links.push_back(
				nav_link{.paragraph = p.id, .type = nav_type_shop, .cost = atol(
					Cost), .monster = {}, .buyable = {.name = ItemName, .flags = Value}, .prompt = "", .answer = "", .label = ""});
		} else {
			if (current_player.possessions.size() < max - 1) {
				p.navigation_links.push_back(
					nav_link{.paragraph = p.id, .type = nav_type_shop, .cost = atol(
						Cost), .monster = {}, .buyable = {.name = ItemName, .flags = Value}, .prompt = "", .answer = "", .label = ""});
			} else {
				p.navigation_links.push_back(
					nav_link{.paragraph = p.id, .type = nav_type_disabled_link, .cost = 0, .monster = {}, .buyable = {}, .prompt = "", .answer = "", .label = "Inventory Full"});
			}
		}
		co_return;
	}
};

static i_tag self_init;
