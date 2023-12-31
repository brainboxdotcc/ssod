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

bool route_tag(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player, bool display) {
	auto ref = registered_tags.find(dpp::lowercase(p_text));
	if (ref != registered_tags.end()) {
		auto ovr = tag_display_override.find(dpp::lowercase(p_text));
		if (!display && ovr == tag_display_override.end()) {
			return true;
		}
		auto ptr = ref->second;
		(*ptr)(p, p_text, paragraph_content, output, current_player);
		return true;
	}
	return false;
}
