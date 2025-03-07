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
#include <ssod/ssod.h>

using namespace i18n;

struct pop_tag : public tag {
	pop_tag() { register_tag<pop_tag>(); }
	static constexpr std::string_view tags[]{"<pop>"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		output << "\n\n**" << tr("RETURN", current_player.event) << "** " << directions[++p.links] << "\n\n";
		p.navigation_links.push_back(nav_link{ .paragraph = p.id, .type = nav_type_pop, .cost = 0, .monster = {}, .buyable = {}, .prompt = "", .answer = "", .label = "" });
		p.words++;
		co_return;
	}
};

static pop_tag self_init;
