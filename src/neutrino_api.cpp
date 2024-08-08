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
#include <string>
#include <ssod/neutrino_api.h>
#include <ssod/sentry.h>

neutrino::neutrino(dpp::cluster* cl, const std::string& userid, const std::string& apikey) : cluster(cl), user_id(userid), api_key(apikey) {
}

void neutrino::contains_bad_word(const std::string& text, swear_filter_event_t callback) {
	std::string request = nlohmann::json({
		{ "content", text },
		{ "censor-character", "#" },
		{ "catalog", "strict" },
		{ "output-format", "JSON" },
	}).dump();
	void* tx = sentry::get_user_transaction();
	void* span = sentry::http_span(tx, "https://neutrinoapi.net/bad-word-filter", "POST");
	this->cluster->request("https://neutrinoapi.net/bad-word-filter", dpp::m_post, [this, text, callback, span](const auto& rv) {
			swear_filter_t sf;
			nlohmann::json j;
			sf.clean = true;
			sf.censored_content = text;
			sentry::end_http_span(span, rv.status);
			if (rv.error == dpp::h_success && !rv.body.empty()) {
				try {
					j = nlohmann::json::parse(rv.body);
					if (j.contains("is-bad") && j.at("is-bad").get<bool>()) {
						sf.clean = false;
						sf.censored_content = j.at("censored-content").get<std::string>();
					}
				}
				catch (const std::exception &e) {
					callback(sf);
				}
			}
			callback(sf);
		},
		request,
		"application/json",
		{
			{ "Content-Type", "application/json" },
			{ "Accept", "application/json" },
			{ "user-id", this->user_id },
			{ "api-key", this->api_key },
		}
	);
}
