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
#include <ssod/parser.h>
#include <ssod/achievement.h>
#include <gen/emoji.h>

using namespace i18n;

struct book_tag : public tag {
	book_tag() { register_tag<book_tag>(); }
	static constexpr std::string_view tags[]{"<book"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string id;
		paragraph_content >> id;
		long book_id{atol(id)};
		/* Check for a translation for the user's locale, if there is one */
		auto rs = co_await db::co_query("SELECT * FROM books WHERE id = ?", { book_id });
		if (rs.empty()) {
			co_return;
		}
		auto book = rs[0];
		std::string book_title = book.at("title");
		std::string book_author = book.at("author");
		std::string book_tags = book.at("tags");
		if (current_player.event.command.locale.substr(0, 2) != "en") {
			auto translated_text = co_await db::co_query(
				"SELECT * FROM translations WHERE table_col IN (?,?) AND row_id = ? AND language = ? ORDER BY table_col",
				{"books/title", "books/author", book_id, current_player.event.command.locale.substr(0, 2)}
			);
			// fixed order: author, title
			if (translated_text.size() == 2) {
				book_author = translated_text[0].at("translation");
				book_title = translated_text[1].at("translation");
			}
		}
		/* Select one random flavour text whose JSON tags overlap the JSON tags of the book */
		auto flavour = co_await db::co_query("SELECT * FROM book_flavour_text WHERE JSON_OVERLAPS(matched_canonical_tags, ?) ORDER BY RAND() LIMIT 1", { book_tags });
		if (flavour.empty()) {
			flavour = co_await db::co_query("SELECT * FROM book_flavour_text ORDER BY RAND() LIMIT 1");
		}
		std::string flavour_text = flavour.at(0).at("flavour_text");
		if (current_player.event.command.locale.substr(0, 2) != "en") {
			auto translated_text = co_await db::co_query(
				"SELECT * FROM translations WHERE table_col = ? AND row_id = ? AND language = ?",
				{"book_flavour_text/flavour_text", flavour.at(0).at("id"), current_player.event.command.locale.substr(0, 2)}
			);
			// fixed order: author, title
			if (!translated_text.empty()) {
				flavour_text = translated_text[0].at("translation");
			}
		}

		output << flavour_text + "\n**" << sprite::book.get_mention() << " " << book_title << "**\nBy " << book_author << "\n\n";

		p.navigation_links.push_back(nav_link{ .paragraph = p.id, .type = nav_type_book, .cost = book_id, .monster = {}, .buyable = {}, .prompt = book_author, .answer = book_title, .label = "" });
		p.words++;


		co_return;
	}
};

static book_tag self_init;
