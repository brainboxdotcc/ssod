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

struct macro_tag : public tag {
	macro_tag() { register_tag<macro_tag>(); }
	static constexpr std::string_view tags[]{"<macro"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		long macro_id{};
		paragraph_content >> macro_id;
		paragraph macro_p = co_await paragraph::create(macro_id, current_player, current_player.event.command.usr.id, p.id);
		output << macro_p.text;
		p.navigation_links.insert(p.navigation_links.end(), macro_p.navigation_links.begin(), macro_p.navigation_links.end());
		co_return;
	}
};

static macro_tag self_init;
