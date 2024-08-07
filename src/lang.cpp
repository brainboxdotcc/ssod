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
#include <ssod/database.h>
#include <ssod/sentry.h>

namespace i18n {

	std::shared_mutex lang_mutex;
	static dpp::interaction_create_t english{};
	time_t last_lang{0};
	json *lang{nullptr};

	time_t get_mtime(const char *path) {
		struct stat stat_buf{};
		if (stat(path, &stat_buf) == -1) {
			return 0;
		}
		return stat_buf.st_mtime;
	}

	void check_lang_reload(dpp::cluster &bot) {
		if (get_mtime("../lang.json") > last_lang) {
			std::unique_lock lang_lock(lang_mutex);
			last_lang = get_mtime("../lang.json");
			std::ifstream langfile("../lang.json");
			json *new_lang = new json();
			try {
				json *old_lang = lang;

				// Parse updated contents
				langfile >> *new_lang;

				lang = new_lang;
				delete old_lang;
			}
			catch (const std::exception &e) {
				bot.log(dpp::ll_error, fmt::format("Error in lang.json: ", e.what()));
				sentry::log_catch(typeid(e).name(), e.what());
				delete new_lang;
			}
		}
	}

	void load_lang(dpp::cluster &bot) {
		std::unique_lock lang_lock(lang_mutex);
		last_lang = get_mtime("../lang.json");
		english.command.locale = "en";
		std::ifstream lang_file("../lang.json");
		lang = new json();
		lang_file >> *lang;
		bot.log(dpp::ll_info, fmt::format("Language strings count: {}", lang->size()));
	}

	std::string tr(const std::string &k, const dpp::interaction_create_t &interaction) {
		std::string lang_name{interaction.command.locale.substr(0, 2)};
		std::shared_lock lang_lock(lang_mutex);
		try {
			auto o = lang->find(k);
			if (o != lang->end()) {
				auto v = o->find(lang_name);
				if (v != o->end()) {
					return v->get<std::string>();
				}
				return tr(k, english);
			}
		}
		catch (const std::exception &e) {
			sentry::log_catch(typeid(e).name(), e.what());
		}
		return k;
	}

	std::string discord_lang(const std::string &l) {
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

	dpp::command_option_choice tr(dpp::command_option_choice choice) {
		auto o = lang->find(choice.name);
		if (o != lang->end()) {
			dpp::interaction_create_t e{};
			for (auto v = o->begin(); v != o->end(); ++v) {
				if (v.key() == "en") {
					continue;
				}
				/* Note: We don't translate the value for choice, this remains constant internally */
				e.command.locale = discord_lang(v.key());
				choice.add_localization(e.command.locale, tr(choice.name, e));
			}
			choice.name = tr(choice.name, english);
		}
		return choice;
	}

	dpp::command_option tr(dpp::command_option opt) {
		auto o = lang->find(opt.name);
		if (o != lang->end()) {
			dpp::interaction_create_t e{};
			for (auto v = o->begin(); v != o->end(); ++v) {
				if (v.key() == "en") {
					continue;
				}
				e.command.locale = discord_lang(v.key());
				opt.add_localization(e.command.locale, tr(opt.name, e), tr(opt.description, e));
			}
			opt.name = tr(opt.name, english);
			opt.description = tr(opt.description, english);
		}
		for (auto &choice: opt.choices) {
			choice = tr(choice);
		}
		for (auto &option: opt.options) {
			option = tr(option);
		}
		return opt;
	}

	item_desc tr(const item &i, const std::string &description, const dpp::interaction_create_t &event) {
		if (event.command.locale.substr(0, 2) == "en") {
			return {.name = i.name, .description = description};
		}
		auto res = db::query("SELECT id FROM game_item_descs WHERE name = ?", {i.name});
		if (res.empty()) {
			/* Not an item desc */
			res = db::query("SELECT id FROM food WHERE name = ?", {i.name});
			if (res.empty()) {
				/* Not food */
				res = db::query("SELECT id FROM ingredients WHERE ingredient_name = ?", {i.name});
				if (!res.empty()) {
					auto translated_text = db::query("SELECT * FROM translations WHERE table_col = ? AND row_id = ? AND language = ? ORDER BY table_col", {
						"ingredients/ingredient_name", res[0].at("id"), event.command.locale.substr(0, 2)
					});
					if (!translated_text.empty()) {
						return {.name = translated_text[0].at("translation"), .description = tr("COOK_ME", event)};
					}
				}
				return {.name = i.name, .description = description};
			}
			/* Is food */
			auto translated_text = db::query("SELECT * FROM translations WHERE (table_col = ? OR table_col = ?) AND row_id = ? AND language = ? ORDER BY table_col", {
				"food/name", "food/description", res[0].at("id"), event.command.locale.substr(0, 2)
			});
			if (translated_text.size() == 2) {
				return {.name = translated_text[1].at("translation"), .description = translated_text[0].at("translation")};
			}
			return {.name = i.name, .description = description};
		}
		/* Is item desc */
		auto translated_text = db::query("SELECT * FROM translations WHERE (table_col = ? OR table_col = ?) AND row_id = ? AND language = ? ORDER BY table_col", {
			"game_item_descs/name", "game_item_descs/idesc", res[0].at("id"), event.command.locale.substr(0, 2)
		});
		if (translated_text.size() != 2) {
			return {.name = i.name, .description = description};
		}
		return {.name = translated_text[1].at("translation"), .description = translated_text[0].at("translation")};
	}

	item_desc tr(const stacked_item &i, const std::string &description, const dpp::interaction_create_t &event) {
		return tr(item{ .name = i.name, .flags = i.flags}, description, event);
	}

	dpp::slashcommand tr(dpp::slashcommand cmd) {
		auto o = lang->find(cmd.name);
		if (o != lang->end()) {
			dpp::interaction_create_t e{};
			for (auto v = o->begin(); v != o->end(); ++v) {
				if (v.key() == "en") {
					continue;
				}
				e.command.locale = discord_lang(v.key());
				cmd.add_localization(e.command.locale, tr(cmd.name, e), tr(cmd.description, e));
			}
			cmd.name = tr(cmd.name, english);
			cmd.description = tr(cmd.description, english);
		}
		for (auto &option: cmd.options) {
			option = tr(option);
		}
		return cmd;
	}

};