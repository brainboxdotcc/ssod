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

std::string sha256(const std::string &buffer);

inline long atol(const std::string& str) {
	if (str.empty()) return 0;
	return atol(str.c_str());
}

inline long long atoll(const std::string& str) {
	if (str.empty()) return 0;
	return atoll(str.c_str());
}

inline int atoi(const std::string& str) {
	if (str.empty()) return 0;
	return atoi(str.c_str());
}

struct item_desc {
	std::string name{};
	std::string description{};
};

struct item {
	std::string name{};
	std::string flags{};
};

namespace i18n {

	std::string _(const std::string &k, const dpp::interaction_create_t &interaction);

	dpp::slashcommand _(dpp::slashcommand cmd);

	item_desc _(const item &i, const std::string &description, const dpp::interaction_create_t &event);

	template<typename... T>
	std::string _(const std::string &key, const dpp::interaction_create_t &interaction, T &&... args) {
		return fmt::format(fmt::runtime(_(key, interaction)), std::forward<T>(args)...);
	}

};

using namespace i18n;
