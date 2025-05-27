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

	db::resultset fetch_translations(const std::vector<std::string>& table_cols, const std::string& row_id, const std::string& lang) {
		if (table_cols.size() == 1) {
			return db::query(
				"SELECT * FROM translations WHERE table_col = ? AND row_id = ? AND language = ? ORDER BY table_col",
				{table_cols[0], row_id, lang}
			);
		}
		return db::query(
			"SELECT * FROM translations WHERE (table_col = ? OR table_col = ?) AND row_id = ? AND language = ? ORDER BY table_col",
			{table_cols[0], table_cols[1], row_id, lang}
		);
	}


	std::optional<item_desc> try_translate_item_desc(const item& i, const std::string& lang) {
		auto res = db::query("SELECT id FROM game_item_descs WHERE name = ?", {i.name});
		if (res.empty()) return std::nullopt;

		auto translated = fetch_translations({"game_item_descs/name", "game_item_descs/idesc"}, res[0].at("id"), lang);

		if (translated.size() == 2) {
			return item_desc{
				.name = translated[1].at("translation"),
				.description = translated[0].at("translation")
			};
		}

		return std::nullopt;
	}


	std::optional<item_desc> try_translate_food(const item& i, const std::string& lang) {
		auto res = db::query("SELECT id FROM food WHERE name = ?", {i.name});
		if (res.empty()) return std::nullopt;

		auto translated = fetch_translations({"food/name", "food/description"}, res[0].at("id"), lang);

		if (translated.size() == 2) {
			return item_desc{
				.name = translated[1].at("translation"),
				.description = translated[0].at("translation")
			};
		}

		return std::nullopt;
	}


	std::optional<item_desc> try_translate_ingredient(const item& i, const std::string& lang, const dpp::interaction_create_t& event) {
		auto res = db::query("SELECT id FROM ingredients WHERE ingredient_name = ?", {i.name});
		if (res.empty()) return std::nullopt;

		auto translated = fetch_translations({"ingredients/ingredient_name"}, res[0].at("id"), lang);

		if (!translated.empty()) {
			return item_desc{
				.name = translated[0].at("translation"),
				.description = tr("COOK_ME", event)
			};
		}

		return std::nullopt;
	}

	std::optional<item_desc> try_translate_book(const item& i, const std::string& lang) {
		auto res = db::query("SELECT id, author FROM books WHERE title = ?", {i.name});
		if (res.empty()) return std::nullopt;

		auto translated = fetch_translations({"books/title"}, res[0].at("id"), lang);

		if (!translated.empty()) {
			return item_desc{
				.name = translated[0].at("translation"),
				.description = res[0].at("author")
			};
		}

		return std::nullopt;
	}


	item_desc tr(const item &i, const std::string &description, const dpp::interaction_create_t &event) {
		const std::string language = event.command.locale.substr(0, 2);

		if (language == "en") {
			return {.name = i.name, .description = description};
		}

		if (auto result = try_translate_item_desc(i, language)) return *result;
		if (auto result = try_translate_food(i, language)) return *result;
		if (auto result = try_translate_ingredient(i, language, event)) return *result;
		if (auto result = try_translate_book(i, language)) return *result;

		return {.name = i.name, .description = description};
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