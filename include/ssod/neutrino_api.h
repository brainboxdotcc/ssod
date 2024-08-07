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
#include <string_view>
#include <functional>
#include <dpp/dpp.h>

/**
 * @brief The results of a censor filter API call
 */
struct swear_filter_t {
        /**
         * @brief True if the text is clean of swearing
         */
        bool clean = true;
        /**
         * @brief The censored content with swearwords replaced by # symbols
         */
        std::string censored_content;
};

/**
 * @brief Censor API callback function
 */
typedef std::function<void(const swear_filter_t&)> swear_filter_event_t;

/**
 * @brief A class for calling Neutrino API censor endpoint
 * The censor endpoint filters swearwords from text in multiple languages.
 * It is maintained by a third party and is much better than any simple
 * list i could create and maintain myself.
 */
class neutrino
{
	/**
	 * @brief Endpoint URL for API
	 */
	static constexpr std::string_view endpoint{"https://neutrinoapi.net/bad-word-filter"};
        /**
         * @brief D++ cluster
         */
        dpp::cluster* cluster;

        /**
         * @brief API user ID
         */
        std::string user_id;

        /**
         * @brief API key
         */
        std::string api_key;
public:
        /**
         * @brief Construct a new neutrino object
         *
         * @param cl D++ Cluster
         * @param userid API user ID
         * @param apikey API Key
         */
        neutrino(dpp::cluster* cl, const std::string& userid, const std::string& apikey);

        /**
         * @brief Returns a swear_filter_t in a callback after checking text for swearing.
         * Uses a HTTPS REST API call.
         *
         * @param text text to check
         * @param callback callback to receive swear filter result
         */
        void contains_bad_word(const std::string& text, swear_filter_event_t callback);

	/**
	 * @brief Returns a swear_filter_t when co_awaited, after checking text for swearing.
	 * Uses a HTTPS REST API call.
	 *
	 * @param text text to check
	 * @param callback callback to receive swear filter result
	 */
	dpp::async<swear_filter_t> co_contains_bad_word(const std::string& text) {
		return dpp::async<swear_filter_t>{ [text, this] <typename C> (C &&cc) { return contains_bad_word(text, std::forward<C>(cc)); }};
	}

};
