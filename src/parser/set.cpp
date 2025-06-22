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
#include <ssod/achievement.h>

struct set_tag : public tag {
	set_tag() { register_tag<set_tag>(); }
	static constexpr std::string_view tags[]{"<set"};

	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		auto args = read_tag_arguments(paragraph_content);
		if (args.empty()) co_return;

		std::string flag = dpp::lowercase(args[0]);
		std::string value = "1";

		if (args.size() > 1) {
			std::string val = dpp::lowercase(args[1]);
			auto scoremap = get_score_map(current_player);
			value = (scoremap.contains(val) ? std::to_string(scoremap[val]) : val);
		}

		std::string full_key = "gamestate_" + flag;
		if (!current_player.has_flag(full_key) || args.size() > 1) {
			current_player.set_flag(full_key, p.id, value);

			if (flag == "steam_copter" && !current_player.has_flag("steamcopter")) {
				current_player.add_flag("steamcopter");
			}

			co_await achievement_check("STATE", current_player.event, current_player, {{"flag", flag }, {"value", value}});
		}

		co_return;
	}
};

static set_tag self_init;