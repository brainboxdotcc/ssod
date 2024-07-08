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
#include <atomic>
#include <cstdint>
#include <fmt/core.h>

namespace fs = std::filesystem;

#define SSOD_VERSION "ssod@4.0.0"

using json = dpp::json;

#define EMBED_COLOUR 0xd5b994

/**
 *  trim from end of string (right)
 */
inline std::string rtrim(std::string s)
{
	s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
	return s;
}

/**
 * trim from beginning of string (left)
 */
inline std::string ltrim(std::string s)
{
	s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
	return s;
}

/**
 * trim from both ends of string (right then left)
 */
inline std::string trim(std::string s)
{
	return ltrim(rtrim(s));
}

/**
 * String to long
 * @param str
 * @return
 */
inline long atol(const std::string& str) {
	if (str.empty()) return 0;
	return atol(str.c_str());
}

/**
 * String to long long
 * @param str
 * @return
 */
inline long long atoll(const std::string& str) {
	if (str.empty()) return 0;
	return atoll(str.c_str());
}

/**
 * String to int
 * @param str
 * @return
 */
inline int atoi(const std::string& str) {
	if (str.empty()) return 0;
	return atoi(str.c_str());
}

/**
 * Represents the description of an item.
 */
struct item_desc {
	std::string name{};
	std::string description{};
};

/**
 * Represents a singular item on the ground, in the bank, or in the
 * player's inventory.
 */
struct item {
	std::string name{};
	std::string flags{};
};

/**
 * Represents a stack of identical items, on the ground, in the bank
 * or in the player's inventory
 */
struct stacked_item {
	std::string name{};
	std::string flags{};
	long qty{1};
};

/**
 * Internationalisation functions
 */
namespace i18n {

	/**
	 * Fetch translation with a given key name, using the language of an interaction's user
	 * @param k translation key
	 * @param interaction user interaction
	 * @return translated text
	 */
	std::string tr(const std::string& k, const dpp::interaction_create_t& interaction);

	/**
	 * Translate a slash command into all possible languages
	 * @param cmd slash command
	 * @return translated slash command
	 */
	dpp::slashcommand tr(dpp::slashcommand cmd);

	/**
	 * Translate name and description of an item into the language of an interaction's user
	 * @param i item
	 * @param description item description
	 * @param event user interaction
	 * @return translated item
	 */
	item_desc tr(const item& i, const std::string &description, const dpp::interaction_create_t& event);

	/**
	 * Translate name and description of an item stack into the language of an interaction's user
	 * @param i item stack
	 * @param description item description
	 * @param event user interaction
	 * @return translated item
	 */
	item_desc tr(const stacked_item& i, const std::string &description, const dpp::interaction_create_t& event);

	/**
	 * Fetch translation with a given key name, using the language of an interaction's user. Accepts optional
	 * fmt::format arguments.
	 * @tparam T variadic type names for argument list
	 * @param k translation key
	 * @param interaction user interaction
	 * @param args variadic argument list
	 * @return translated string
	 */
	template<typename... T>
	std::string tr(const std::string& key, const dpp::interaction_create_t& interaction, T &&... args) {
		try {
			return fmt::format(fmt::runtime(tr(key, interaction)), std::forward<T>(args)...);
		}
		catch (const std::exception& format_exception) {
			if (interaction.from && interaction.from->creator) {
				interaction.from->creator->log(dpp::ll_error, "Error in translation string for translation " + key + " lang " + interaction.command.locale + ": " + format_exception.what());
			}
			return key;
		}
	}

};

