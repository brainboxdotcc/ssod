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
#include <dpp/dpp.h>
#include <ssod/book_reader.h>
#include <ssod/component_builder.h>
#include <ssod/game_util.h>
#include <ssod/ssod.h>
#include <gen/emoji.h>
#include <ssod/aes.h>
#include <fmt/format.h>

using namespace i18n;

dpp::task<void> continue_book(const dpp::interaction_create_t& event, player &p) {
	dpp::cluster& bot = *(event.owner);
	std::stringstream content;

	auto book = co_await db::co_query("SELECT * FROM books WHERE id = ?", {p.reading_book_id});
	auto page = co_await db::co_query("SELECT * FROM book_pages WHERE book_id = ? AND page_index = ?", {p.reading_book_id, p.book_page});
	auto max_page_query = co_await db::co_query("SELECT MAX(page_index) AS max_page FROM book_pages WHERE book_id = ?", {p.reading_book_id});

	if (book.empty() || page.empty()) {
		/* Invalid book ID or page number */
		bot.log(dpp::ll_debug, event.command.usr.id.str() + ": invalid book id " + std::to_string(p.reading_book_id) + " or missing page; bug, or hack attempt");
		co_return;
	}

	long max_page = atol(max_page_query.at(0).at("max_page"));

	item book_item{ .name = book.at(0).at("title"), .flags = "B" + book.at(0).at("id") };
	item_desc i = tr(book_item, "", event);

	std::string page_text = page.at(0).at("content");
	std::string language{event.command.locale.substr(0, 2)};
	if (language != "en") {
		auto translated = co_await db::co_query("SELECT * FROM translations WHERE row_id = ? AND table_col = ? AND language = ?", {page.at(0).at("id"), "book_pages/content", language});
		if (!translated.empty()) {
			page_text = translated.at(0).at("translation");
		}
	}

	content << "**__" << i.name << "__**\n";
	content << "*" <<book.at(0).at("author") << "*\n\n";
	content << page_text << "\n\n";

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{
			.text = tr("PAGE_NAV", event, p.book_page + 1, max_page + 1),
			.icon_url = bot.me.get_avatar_url(),
			.proxy_url = "",
		})
		.set_thumbnail("https://images.ssod.org/resource/book" + std::to_string(p.reading_book_id % 6) + ".png")
		.set_colour(EMBED_COLOUR)
		.set_description(content.str());

	dpp::message m;
	component_builder cb(m);

	cb.add_component(dpp::component()
		 .set_type(dpp::cot_button)
		 .set_id(security::encrypt("exit_book"))
		 .set_label(tr("BACK", event))
		 .set_style(dpp::cos_primary)
		 .set_emoji(sprite::magic05.name, sprite::magic05.id)
	);
	cb.add_component(dpp::component()
		 .set_type(dpp::cot_button)
		 .set_id(security::encrypt("book_page;" + std::to_string(p.book_page - 1)))
		 .set_style(dpp::cos_secondary)
		 .set_emoji("◀\uFE0F")
		 .set_disabled(p.book_page <= 0)
	);
	cb.add_component(dpp::component()
		 .set_type(dpp::cot_button)
		 .set_id(security::encrypt("book_page;" + std::to_string(p.book_page + 1)))
		 .set_style(dpp::cos_secondary)
		 .set_emoji("▶")
		 .set_disabled(p.book_page >= max_page)
	);
	cb.add_component(help_button(event));
	m = cb.get_message();
	m.embeds = { embed };

	event.reply(event.command.type == dpp::it_application_command ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying book:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
		}
	});

	co_return;
}


dpp::task<bool> book_nav(const dpp::interaction_create_t& event, player &p, const std::vector<std::string>& parts) {

	dpp::cluster& bot = *(event.owner);

	if (parts[0] == "exit_book" && parts.size() == 1) {
		p.reading_book_id = 0;
		bot.log(dpp::ll_debug, "CLOSE BOOK");
		co_return true;
	} else if (parts[0] == "book_page" && parts.size() == 2) {
		p.book_page = atol(parts[1]);
		bot.log(dpp::ll_debug, "BOOK PAGE " + parts[1]);
		co_return true;
	}

	co_return false;

}