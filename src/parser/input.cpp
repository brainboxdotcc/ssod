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

struct input_tag : public tag {
	input_tag() { register_tag<input_tag>(); }
	static constexpr std::string_view tags[]{"<input"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;
		extract_to_quote(p_text, paragraph_content, '"');
		std::string Prompt = extract_value(p_text);
		paragraph_content >> p_text;
		long destination = extract_value_number(p_text);
		paragraph_content >> p_text;
		std::string Correct = extract_value(p_text);
		output << "\n### ❓ " << Prompt << "\n\n";
		++p.links;
		p.navigation_links.push_back(nav_link{ .paragraph = destination, .type = nav_type_modal, .cost = 0, .monster = {}, .buyable = {}, .prompt = Prompt, .answer = Correct, .label = "" });
		p.words++;
		co_return;
	}
};

static input_tag self_init;
