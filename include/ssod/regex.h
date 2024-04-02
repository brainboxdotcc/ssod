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

/**
 * An exception thrown by a regular expression
 */
class regex_exception : public std::exception {
public:
	std::string message;
	explicit regex_exception(std::string _message);
};

/**
 * Class pcre_regex represents a perl compatible regular expression
 * This is internally managed by libpcre.
 * Regular expressions are compiled in the constructor and matched
 * by the Match() methods.
 */
class pcre_regex
{
	const char* pcre_error;
	int pcre_error_ofs;
	pcre* compiled_regex;
	int* match_arr;
	int max_matches;
 public:
	/* Constructor */
	explicit pcre_regex(const std::string &match, bool case_insensitive = false, int max_match = 30);
	~pcre_regex();
	/* Match methods */
	bool match(const std::string &comparison);
	bool match(const std::string &comparison, std::vector<std::string>& matches);
	std::string replace(std::string comparison, const std::string &replace);
	std::string replace_all(std::string comparison, const std::string &replace);
};

