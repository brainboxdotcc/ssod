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
#pragma once
#include <dpp/dpp.h>
#include <ssod/game_player.h>
#include <ssod/paragraph.h>
#include <set>

/**
 * @brief Tags used in the parser for the game's content language
 * @note Tags self-register via static initialisation
 */
struct tag {

	/**
	 * If true, this tag will be called even when the parser is not currently displaying content.
	 * In short, the parser will never skip these tags. Used mainly for conditionals.
	 */
	static constexpr bool overrides_display{false};

	/**
	 * Names of tags in the content, without trailing > symbol
	 */
	static constexpr std::string_view tags[]{};

	/**
	 * Handle the tag
	 * @param p current paragraph
	 * @param p_text paragraph raw text
	 * @param paragraph_content paragraph input content
	 * @param output output text content
	 * @param current_player current player
	 */
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player);
};

/**
 * @brief A function pointer to the static route() function of a tag
 */
using tag_router = auto (*)(paragraph&, std::string&, std::stringstream&, std::stringstream&, player&) -> dpp::task<void>;

/**
 * @brief Represents a list of registered tags stored in an unordered_map
 */
using registered_tag_list = std::unordered_map<std::string_view, tag_router>;

/**
 * @brief Represents a list of registered tags that can override display
 */
using registered_over_list = std::set<std::string>;

registered_tag_list& get_tag_map();
registered_over_list& get_over_set();

template <typename T> void register_tag()
{
	auto& tag_map = get_tag_map();
	auto& over_set = get_over_set();
	for (auto r : T::tags) {
		tag_map[r] = T::route;
		if (T::overrides_display) {
			over_set.emplace(r);
		}
	}
}

/**
 * Thrown when parsing should immediately end regardless of what is left in the input
 */
struct parse_end_exception : public std::exception {
};

dpp::task<bool> route_tag(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player, bool display);

std::unordered_map<std::string, std::string> parse_attributes(std::stringstream& ss);