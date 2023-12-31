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
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// <input prompt="prompt" location="loc_id" value="correct_answer">
		// TODO: Modal Dialog!
		p.links++;
		paragraph_content >> p_text;
		extract_to_quote(p_text, paragraph_content, '"');
		std::string Prompt = extract_value(p_text);
		paragraph_content >> p_text;
		std::string Para = extract_value(p_text);
		paragraph_content >> p_text;
		std::string Correct = extract_value(p_text);
		output << "\n\n";
		//output << "<b>" << Prompt << "</b><br><form action='" << me << "'><input type='hidden' name='action' value='riddle'><input type='hidden' name='guid' value='" << formData[1] << "'><input type='hidden' name='keycode' value='" << Key << "'><input type='text' name='q' value='""'><input type='hidden' name='p' value='" << Para << "'><input type='submit' value='Answer'></form>" << CR << CR;
		output << "TODO: Input Dialog\n";
		p.words++;
	}
};

static input_tag self_init;
