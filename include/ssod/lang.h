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

#include <sys/stat.h>
#include <sys/types.h>
#include <dpp/dpp.h>
#include <optional>
#include <ssod/database.h>

namespace i18n {

	time_t get_mtime(const char *path);

	void load_lang(dpp::cluster &bot);

	void check_lang_reload(dpp::cluster &bot);

	std::optional<item_desc> try_translate_book(const item& i, const std::string& lang);

	std::optional<item_desc> try_translate_food(const item& i, const std::string& lang);

	std::optional<item_desc> try_translate_ingredient(const item& i, const std::string& lang);

	std::optional<item_desc> try_translate_item_desc(const item& i, const std::string& lang);

	db::resultset fetch_translations(const std::vector<std::string>& table_cols, const std::string& row_id, const std::string& lang);
}