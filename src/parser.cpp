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
#include <dpp/dpp.h>
#include <ssod/parser.h>

/**
 * @brief Internal command map
 */
static registered_tag_list registered_tags;
static registered_over_list tag_display_override;

registered_tag_list& get_tag_map()
{
	return registered_tags;
}

registered_over_list& get_over_set()
{
	return tag_display_override;
}

dpp::task<bool> route_tag(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player, bool display) {
	auto ref = registered_tags.find(dpp::lowercase(p_text));
	if (ref != registered_tags.end()) {
		auto ovr = tag_display_override.find(dpp::lowercase(p_text));
		if (!display && ovr == tag_display_override.end()) {
			co_return true;
		}
		auto ptr = ref->second;
		co_await (*ptr)(p, p_text, paragraph_content, output, current_player);
		co_return true;
	}
	co_return false;
}

std::unordered_map<std::string, std::string> parse_attributes(std::stringstream& ss) {
	std::unordered_map<std::string, std::string> attributes;
	std::string token;

	while (ss >> token) {
		if (token == ">") {
			break;
		}

		bool ends_with_gt = false;
		if (!token.empty() && token.back() == '>') {
			ends_with_gt = true;
			token.pop_back(); // remove '>'
		}

		auto eq_pos = token.find('=');
		if (eq_pos == std::string::npos) {
			attributes[dpp::lowercase(token)] = "";
		} else {
			std::string key = token.substr(0, eq_pos);
			std::string value = token.substr(eq_pos + 1);

			if (!value.empty() && value[0] == '"') {
				std::string quoted = value;
				if (quoted.back() != '"') {
					std::string fragment;
					while (ss >> fragment) {
						quoted += ' ' + fragment;
						if (!fragment.empty() && fragment.back() == '"') {
							break;
						}
					}
				}
				if (quoted.size() >= 2 && quoted.front() == '"' && quoted.back() == '"') {
					value = quoted.substr(1, quoted.size() - 2);
				}
			}

			attributes[dpp::lowercase(key)] = value;
		}

		if (ends_with_gt) {
			break;
		}
	}

	return attributes;
}

bool try_parse_long(const std::string& s, long& result) {
	try {
		size_t idx;
		result = std::stol(s, &idx);
		return idx == s.size();
	} catch (...) {
		return false;
	}
}

std::vector<std::string> read_tag_arguments(std::stringstream& in) {
	std::vector<std::string> args;
	std::string token;
	while (in >> token) {
		if (token.ends_with('>')) {
			token.pop_back(); // remove '>'
			args.push_back(token);
			break;
		}
		args.push_back(token);
	}
	return args;
}

std::map<std::string, long> get_score_map(const player& current_player) {
	std::map<std::string, long> defaults = {
		{ "exp", current_player.experience },
		{ "dice", current_player.g_dice },
		{ "stm", current_player.stamina },
		{ "skl", current_player.skill },
		{ "arm", current_player.armour.rating },
		{ "wpn", current_player.weapon.rating },
		{ "day", current_player.days },
		{ "spd", current_player.speed },
		{ "luck", current_player.luck },
		{ "scrolls", current_player.scrolls },
		{ "level", current_player.get_level() },
		{ "mana", current_player.mana },
		{ "notoriety", current_player.notoriety },
		{ "gold", current_player.gold },
		{ "silver", current_player.silver },
		{ "rations", current_player.rations }
	};
	for (uint8_t reg_no = 0; reg_no < 32; ++reg_no) {
		defaults.emplace("reg" + std::to_string(reg_no), current_player.regs[reg_no]);
	}
	return defaults;
}

