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
#include <fmt/format.h>
#include <dpp/dpp.h>
#include <ssod/game_player.h>
#include <ssod/database.h>

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

std::string discord_lang(const std::string& l) {
	if (l == "es") {
		return "es-ES";
	} else if (l == "pt") {
		return "pt-BR";
	} else if (l == "sv") {
		return "sv-SE";
	} else if (l == "zh") {
		return "zh-CN";
	}
	return l;
};

dpp::command_option_choice _(dpp::command_option_choice choice) {
	auto o = lang->find(choice.name);
	if (o != lang->end()) {
		dpp::interaction_create_t e{};
		for (auto v = o->begin(); v != o->end(); ++v) {
			if (v.key() == "en") {
				continue;
			}
			/* Note: We don't translate the value for choice, this remains constant internally */
			e.command.locale = discord_lang(v.key());
			choice.add_localization(e.command.locale, _(choice.name, e));
		}
		choice.name = _(choice.name, english);
	}
	return choice;
}

dpp::command_option _(dpp::command_option opt) {
	auto o = lang->find(opt.name);
	if (o != lang->end()) {
		dpp::interaction_create_t e{};
		for (auto v = o->begin(); v != o->end(); ++v) {
			if (v.key() == "en") {
				continue;
			}
			e.command.locale = discord_lang(v.key());
			opt.add_localization(e.command.locale, _(opt.name, e), _(opt.description, e));
		}
		opt.name = _(opt.name, english);
		opt.description = _(opt.description, english);
	}
	for (auto & choice : opt.choices) {
		choice = _(choice);
	}
	for (auto & option : opt.options) {
		option = _(option);
	}
	return opt;
}

item_desc _(const item& i, const std::string& description, const dpp::interaction_create_t& event) {
	if (event.command.locale.substr(0, 2) == "en") {
		return { .name = i.name, .description = description };
	}
	auto res = db::query("SELECT id FROM game_item_descs WHERE name = ?", {i.name});
	if (res.empty()) {
		return { .name = i.name, .description = description };
	}
	auto translated_text = db::query("SELECT * FROM translations WHERE (table_col = ? OR table_col = ?) AND row_id = ? AND language = ? ORDER BY table_col", {
		"game_item_descs/name", "game_item_descs/idesc", res[0].at("id"), event.command.locale.substr(0, 2)
	});
	if (translated_text.size() != 2) {
		return { .name = i.name, .description = description };
	}

	return { .name = translated_text[1].at("translation"), .description = translated_text[0].at("translation") };
}

dpp::slashcommand _(dpp::slashcommand cmd) {
	auto o = lang->find(cmd.name);
	if (o != lang->end()) {
		dpp::interaction_create_t e{};
		for (auto v = o->begin(); v != o->end(); ++v) {
			if (v.key() == "en") {
				continue;
			}
			e.command.locale = discord_lang(v.key());
			cmd.add_localization(e.command.locale, _(cmd.name, e), _(cmd.description, e));
		}
		cmd.name = _(cmd.name, english);
		cmd.description = _(cmd.description, english);
	}
	for (auto & option : cmd.options) {
		option = _(option);
	}
	return cmd;
}