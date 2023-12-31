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

struct pickup_tag : public tag {
	pickup_tag() { register_tag<pickup_tag>(); }
	static constexpr std::string_view tags[]{"<pickup"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string item_name, item_flags, flags;

		paragraph_content >> p_text;

		if (dpp::lowercase(p_text) == "scroll>") {
			if (!current_player.has_flag("SCROLL", p.id)) {
				current_player.scrolls++;
				current_player.add_flag("SCROLL", p.id);
			}
			return;
		}

		if (dpp::lowercase(p_text) == "gold") {
			paragraph_content >> p_text;
			p_text = remove_last_char(p_text);
			current_player.add_gold(atoi(p_text.c_str()));
			return;
		}

		if (dpp::lowercase(p_text) == "silver") {
			paragraph_content >> p_text;
			p_text = remove_last_char(p_text);
			current_player.add_silver(atoi(p_text.c_str()));
			return;
		}

		item_name = p_text;
		item_flags = "[[none]]";

		while (p_text.length() && *p_text.rbegin() != '>') {
			paragraph_content >> p_text;
			if (p_text.length() && p_text[0] != '[') {
				item_name += " " + p_text;
			} else {
				item_flags = p_text;
				item_flags = remove_last_char(item_flags);
			}
		}
		if (item_name.length() && *item_name.rbegin() == '>') {
			item_name = remove_last_char(item_name);
		}
		// strip the [ and ] from the item flags...
		for (size_t i = 1; i < item_flags.length() - 1; ++i) {
			flags += item_flags[i];
		}

		if (current_player.has_flag(item_name, p.id)) {
			// crafty player trying to get the same item twice! Not good if its unique!
			return;
		}
		current_player.add_flag(item_name, p.id);
		if (flags == "SPELL") {
			current_player.spells.push_back(item{ .name = item_name, .flags = flags });
		} else if (flags == "HERB") {
			current_player.herbs.push_back(item{ .name = item_name, .flags = flags });
		} else {
			current_player.possessions.push_back(item{ .name = item_name, .flags = flags });
		}
		current_player.inv_change = true;
		return;
	}
};

static pickup_tag self_init;
