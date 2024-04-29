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
#include <ssod/ssod.h>
#include <ssod/lang.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fmt/format.h>
#include <dpp/dpp.h>

std::shared_mutex lang_mutex;
static dpp::interaction_create_t english{};
time_t last_lang{0};
json* lang{nullptr};

time_t get_mtime(const char *path)
{
	struct stat stat_buf{};
	if (stat(path, &stat_buf) == -1) {
		return 0;
	}
	return stat_buf.st_mtime;
}

void check_lang_reload(dpp::cluster& bot) {
	if (get_mtime("../lang.json") > last_lang) {
		std::unique_lock lang_lock(lang_mutex);
		last_lang = get_mtime("../lang.json");
		std::ifstream langfile("../lang.json");
		json* new_lang = new json();
		try {
			json* old_lang = lang;

			// Parse updated contents
			langfile >> *new_lang;

			lang = new_lang;
			delete old_lang;
		}
		catch (const std::exception &e) {
			bot.log(dpp::ll_error, fmt::format("Error in lang.json: ", e.what()));
			delete new_lang;
		}
	}
}

void load_lang(dpp::cluster& bot) {
	std::unique_lock lang_lock(lang_mutex);
	last_lang = get_mtime("../lang.json");
	english.command.locale = "en";
	std::ifstream lang_file("../lang.json");
	lang = new json();
	lang_file >> *lang;
	bot.log(dpp::ll_info, fmt::format("Language strings count: {}", lang->size()));
}

std::string _(const std::string &k, const dpp::interaction_create_t& interaction) {
	std::string lang_name{interaction.command.locale.substr(0, 2)};
	std::shared_lock lang_lock(lang_mutex);
	auto o = lang->find(k);
	if (o != lang->end()) {
		auto v = o->find(lang_name);
		if (v != o->end()) {
			return v->get<std::string>();
		}
		return _(k, english);
	}
	return k;
}
