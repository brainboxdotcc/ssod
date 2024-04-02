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

#include <ssod/regex.h>
#include <pcre.h>
#include <string>
#include <vector>

/**
* @brief Flags used for keeping expressions sane
*/
static pcre_extra flags{
	.flags = PCRE_EXTRA_MATCH_LIMIT | PCRE_EXTRA_MATCH_LIMIT_RECURSION,
	.study_data = nullptr,
	.match_limit = 100,
	.callout_data = nullptr,
	.tables = nullptr,
	.match_limit_recursion = 100,
	.mark = nullptr,
	.executable_jit = nullptr,
};

/**
 * Constructor for PCRE regular expression. Takes an expression to match against and optionally a boolean to
 * indicate if the expression should be treated as case sensitive (defaults to false).
 * Construction compiles the regex, which for a well formed regex may be more expensive than matching against a string.
 */
pcre_regex::pcre_regex(const std::string &match, bool case_insensitive, int max_match) : pcre_error(nullptr), pcre_error_ofs(0) {
	compiled_regex = pcre_compile(match.c_str(), case_insensitive ? PCRE_CASELESS | PCRE_MULTILINE : PCRE_MULTILINE, &pcre_error, &pcre_error_ofs, nullptr);
	if (!compiled_regex) {
		throw regex_exception(pcre_error);
	}
	if (max_match > 0) {
		match_arr.reserve(max_match * 3);
		match_arr.resize(max_match * 3);
	}
}

/**
 * Match regular expression against a string, returns true on match, false if no match.
 */
bool pcre_regex::match(const std::string &comparison) {
	return (pcre_exec(compiled_regex, &flags, comparison.c_str(), comparison.length(), 0, 0, nullptr, 0) > -1);
}

/**
 * Match regular expression against a string, and populate a referenced vector with an
 * array of matches, formatted pretty much like PHP's preg_match().
 * Returns true if at least one match was found, false if the string did not match.
 */
bool pcre_regex::match(const std::string &comparison, std::vector<std::string>& matches) {
	matches.clear();
	auto match_count = pcre_exec(compiled_regex, &flags, comparison.c_str(), (int)comparison.length(), 0, 0,
		match_arr.empty() ? nullptr : match_arr.data(), (int)match_arr.size());
	if (match_count == 0) {
		throw regex_exception("Too many matches");
	}
	for (auto i = 0; i < match_count; ++i) {
		/* Ugly char ops */
		matches.emplace_back(
			comparison.c_str() + match_arr[2 * i],
			(size_t)(match_arr[2 * i + 1] - match_arr[2 * i])
		);
	}
	return match_count > 0;
}

std::string pcre_regex::replace_all(std::string comparison, const std::string &replace) {
	std::string old;
	if (comparison.empty()) {
		return comparison;
	}
	do {
		old = comparison;
		comparison = this->replace(comparison, replace);
	} while (!comparison.empty() && comparison != old);
	return comparison;
}

std::string pcre_regex::replace(std::string comparison, const std::string &replace) {
	if (comparison.empty()) {
		return comparison;
	}
	auto match_count = pcre_exec(compiled_regex, &flags, comparison.c_str(), (int)comparison.length(), 0, 0,
		match_arr.empty() ? nullptr : match_arr.data(), (int)match_arr.size());
	if (match_count == 0) {
		throw regex_exception("Too many matches");
	}
	for (auto i = 0; i < match_count; ++i) {
		/* Ugly char ops */
		auto start = (std::string::size_type)match_arr[2 * i];
		auto length = (std::string::size_type)(match_arr[2 * i + 1] - match_arr[2 * i]);
		try {
			comparison.replace(start, length, replace);
		}
		catch (const std::exception&) {
			throw regex_exception("Replacement out of range of string length (PCRE error)");
		}
	}
	return comparison;
}

/**
 * Destructor to free compiled regular expression structure
 */
pcre_regex::~pcre_regex()
{
	/* Ugh, C libraries */
	free(compiled_regex);
}

