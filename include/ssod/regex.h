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
#include <string>
#include <vector>
#include <exception>
#include <pcre.h>
#include <dpp/dpp.h>

/**
 * An exception thrown by a regular expression.
 * This uses some dpp internals and a macro to build a class derived from dpp::exception.
 */
using exception_error_code = dpp::exception_error_code;
derived_exception(regex_exception, exception);

/**
 * Class pcre_regex represents a perl compatible regular expression.
 *
 * This is internally managed by libpcre.
 *
 * Regular expressions are compiled in the constructor and matched
 * by the match(), replace() and replace_all() methods.
 */
class pcre_regex
{
	/**
	 * @brief Pointer to PCRE error message
	 */
	const char* pcre_error;

	/**
	 * @brief PCRE error offset in expression string
	 */
	int pcre_error_ofs;

	/**
	 * @brief Pointer to compiled PCRE
	 */
	pcre* compiled_regex;

	/**
	 * @brief Array of match indices, this is always a multiple
	 * of three elements because PCRE says so.
	 */
	std::vector<int> match_arr;

 public:
	/**
	 * @brief Constructor for PCRE
	 * @param match Regular expression, you can use R() literals to make these easier to read
	 * @param case_insensitive True to make the pattern matching case insensitive
	 * @param max_match Maximum total matches in the expression that will be allowed. More allowed
	 * matches means more memory usage.
	 * @throws regex_exception on failure to allocate PCRE or parse/compile regular expression
	 */
	explicit pcre_regex(const std::string &match, bool case_insensitive = false, int max_match = 30);

	/**
	 * @brief Destructor for freeing C allocated PCRE
	 */
	~pcre_regex();

	/**
	 * @brief Match regular expression against a string
	 * @param comparison String to compare
	 * @return True if any matches, false if not found
	 */
	bool match(const std::string &comparison);

	/**
	 * @brief Match regular expression against string, fill vector with matches
	 * @param comparison String to compare
	 * @param matches Vector of matches found in string
	 * @return True if there are any matches, false if not found
	 * @throws regex_exception on too many matches
	 */
	bool match(const std::string &comparison, std::vector<std::string>& matches);

	/**
	 * @brief Replace first match of a regular expression in a string
	 * @param comparison String to compare
	 * @param replace String to replace with where matched
	 * @return Replaced string
	 * @throws regex_exception on too many matches
	 */
	std::string replace(std::string comparison, const std::string &replace);

	/**
	 * @brief Replace all matches of a regular expression in a string
	 * @param comparison String to compare
	 * @param replace String to replace with where matched
	 * @return Replaced string
	 * @throws regex_exception on too many matches
	 */
	std::string replace_all(std::string comparison, const std::string &replace);
};
