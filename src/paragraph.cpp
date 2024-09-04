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
#include <ssod/ssod.h>
#include <ssod/paragraph.h>
#include <ssod/database.h>
#include <ssod/game_player.h>
#include <ssod/parser.h>
#include <ssod/wildcard.h>

using namespace i18n;

paragraph::paragraph(uint32_t paragraph_id, player& current, dpp::snowflake user_id, uint32_t parent_id) : id(paragraph_id), cur_player(&current), parent(parent_id) {
	display.push_back(true);
}

dpp::task<paragraph> paragraph::create(uint32_t paragraph_id, player& current, dpp::snowflake user_id, uint32_t parent_id) {
	paragraph new_paragraph(paragraph_id, current, user_id, parent_id);
	co_await new_paragraph.parse(current, user_id);
	co_return new_paragraph;
}

dpp::task<paragraph> paragraph::create(const std::string& data, player& current) {
	paragraph new_paragraph(data, current);
	co_await new_paragraph.parse(current, current.event.command.usr.id);
	co_return new_paragraph;
}

paragraph::paragraph(const std::string& data, player& current) : id(0), text(data), combat_disabled(false), magic_disabled(false), theft_disabled(false), chat_disabled(false), cur_player(&current) {
}

// extracts from any NAME=Value pair
std::string extract_without_quotes(const std::string& p_text) {
	std::string item_name;
	bool copying{false};
	for (const char c : p_text) {
		if (c == '=' || c == ' ' || c == '>') {
			copying = !copying;
			continue;
		}
		if (copying) {
			item_name += c;
		}
	}
	return item_name;
}

dpp::task<bool> paragraph::valid_next(long current, long next) {
	std::set<long> paralist{current};
	std::string p_text;
	auto location = co_await db::co_query("SELECT * FROM game_locations WHERE id = ?", {current});
	if (location.empty()) {
		co_return false;
	}
	std::stringstream paragraph_content(location[0].at("data") + "\r\n");

	while (!paragraph_content.eof()) {
		paragraph_content >> p_text;

		if (paragraph_content.eof()) {
			break;
		}

		std::string tag = dpp::utility::utf8substr(dpp::lowercase(p_text), 0, 20);
		if (tag.find("<paylink=") != std::string::npos) {
			int i{0};
			std::string pnum, cost;
			while (p_text[i++] != '=');
			while (p_text[i] != ',') {
				cost += p_text[i++];
			}
			i++;
			while (p_text[i] != '>') {
				pnum += p_text[i++];
			}
			paralist.insert(atol(pnum));
		} else if (tag.find("<link=") != std::string::npos) {
			int i{0};
			std::string pnum;
			while (p_text[i++] != '=');
			while (p_text[i] != '>' && p_text[i] != ',') {
				pnum += p_text[i++];
			}
			paralist.insert(atol(pnum));
		} else if (tag.find("<autolink=") != std::string::npos) {
			int i{0};
			std::string pnum;
			while (p_text[i++] != '=');
			while (p_text[i] != '>') {
				pnum += p_text[i++];
			}
			paralist.insert(atol(pnum));
		}
	}
	co_return paralist.find(next) != paralist.end();
}

// extracts a value from any NAME="Value" pair
std::string extract_value(const std::string& p_text) {
	if (p_text.find("\"") == std::string::npos) {
		return extract_without_quotes(p_text);
	}
	std::string item_name;
	bool copying{false};
	for (const char c : p_text) {
		if (c == '"') {
			copying = !copying;
			continue;
		}
		if (copying) {
			item_name += c;
		}
	}
	return item_name;
}

long extract_value_number(const std::string& p_text)
{
	return atol(extract_value(p_text));
}

dpp::task<bool> global_set(const std::string& flag) {
	co_return !(co_await db::co_query("SELECT flag FROM game_global_flags WHERE flag = ?", {flag})).empty();
}

dpp::task<bool> timed_set(dpp::interaction_create_t& event, const std::string& flag) {
	bool is_set = !(co_await db::co_query("SELECT id FROM timed_flags WHERE user_id = ? AND flag = ? AND UNIX_TIMESTAMP() < expiry", {event.command.usr.id, flag})).empty();
	co_await db::co_query("DELETE FROM timed_flags WHERE user_id = ? AND flag = ? AND UNIX_TIMESTAMP() >= expiry", {event.command.usr.id, flag});
	co_return is_set;
}


void extract_to_quote(std::string& p_text, std::stringstream& content, char end) {
	while (!content.eof() && p_text.length() && *p_text.rbegin() != end) {
		std::string extra;
		content >> extra;
		p_text += " " + extra;
	}							
}

std::string remove_last_char(const std::string& s) {
	return dpp::utility::utf8substr(s, 0, dpp::utility::utf8len(s) - 1);
}

dpp::task<void> paragraph::parse(player& current_player, dpp::snowflake user_id) {

	/* BUG: g++14.1 ICE: Can't pass members of `this` to an initialiser list within a coroutine */
	uint32_t paragraph_id{id};
	auto location = co_await db::co_query("SELECT * FROM game_locations WHERE id = ?", {paragraph_id});
	if (!location.empty()) {
		/* Check for a translation for the user's locale, if there is one */
		if (current_player.event.command.locale.substr(0, 2) != "en") {
			auto translated_text = co_await db::co_query("SELECT * FROM translations WHERE table_col = ? AND row_id = ? AND language = ?", {"game_locations/data", paragraph_id, current_player.event.command.locale.substr(0, 2)});
			if (!translated_text.empty()) {
				text = translated_text[0].at("translation");
			} else {
				text = location[0].at("data");
			}
		} else {
			text = location[0].at("data");
		}
		secure_id = location[0].at("secure_id");
		combat_disabled = location[0].at("combat_disabled") == "1";
		magic_disabled = location[0].at("magic_disabled") == "1";
		theft_disabled = location[0].at("theft_disabled") == "1";
		chat_disabled = location[0].at("chat_disabled") == "1";
		auto dropped = co_await db::co_query("SELECT item_desc, item_flags, count(item_desc) as stack_count FROM game_dropped_items WHERE location_id = ? GROUP BY item_desc, item_flags ORDER BY item_desc, item_flags LIMIT 50", {paragraph_id});
		for (const auto &dropped_item: dropped) {
			dropped_items.push_back(stacked_item{.name = dropped_item.at("item_desc"), .flags = dropped_item.at("item_flags"), .qty = atol(dropped_item.at("stack_count"))});
		}
	}

	std::stringstream paragraph_content(replace_string(text, "><", "> <") + "\r\n<br>\r\n");
	std::string p_text, LastLink;
	output = new std::stringstream();

	while (!paragraph_content.eof()) {
		paragraph_content >> p_text;
		std::string neat_version{p_text};

		if (paragraph_content.eof()) {
			break;
		}

		try {	
			if (co_await route_tag(*this, p_text, paragraph_content, *this->output, current_player, display.size() ? display[display.size() - 1] : true)) {
				continue;
			}
		}
		catch (const parse_end_exception&) {
			/* If a tag router throws this, it means we are to break out of the parser loop 
			 * and end parsing the content at this tag
			 */
			break;
		}

		if (current_fragment < current_player.after_fragment) {
			/* nothing should be displayed that comes before the desired fragment! */
			continue;
		}

		if (display.size() ? display[display.size() - 1] : true) {
			std::string tag = dpp::lowercase(dpp::utility::utf8substr(p_text, 0, 20));
			if (tag.find("<paylink=") != std::string::npos && !last_was_link) {
				int i{0};
				std::string pnum, cost;
				while (p_text[i++] != '=');
				while (p_text[i] != ',') {
					cost += p_text[i++];
				}
				i++;
				while (p_text[i] != '>') {
					pnum += p_text[i++];
				}
				links++;
				*output << directions[links];
				if (current_player.gold < atol(cost)) {
					navigation_links.push_back(nav_link{ .paragraph = atol(pnum), .type = nav_type_disabled_link, .cost = 0, .monster = {}, .buyable = {}, .prompt = "", .answer = "", .label = "" });
				} else {
					navigation_links.push_back(nav_link{ .paragraph = atol(pnum), .type = nav_type_paylink, .cost = atol(cost), .monster = {}, .buyable = {}, .prompt = "", .answer = "", .label = "" });
				}
				*output << " ";
			} else if (tag.find("<link=") != std::string::npos && !last_was_link) {
				int i{0};
				std::string pnum, label;
				if (p_text.find(',') != std::string::npos) {
					while (p_text[i++] != '=');
					while (p_text[i] != ',') {
						pnum += p_text[i++];
					}
					p_text = dpp::utility::utf8substr(p_text, i + 1, dpp::utility::utf8len(p_text));
					bool bail{false};
					do {
						if (p_text.find('>') == std::string::npos) {
							label += p_text + " ";
							paragraph_content >> p_text;
						} else {
							label += p_text.substr(0, p_text.find('>')) + " ";
							bail = true;
						}
					} while (!bail);
				} else {
					while (p_text[i++] != '=');
					while (p_text[i] != '>') {
						pnum += p_text[i++];
					}
					label = tr("TRAVEL", current_player.event);
				}
				if (current_player.stamina < 1 || dpp::lowercase(pnum) == "the") {
					*output << " " << tr("CANTFOLLOW", current_player.event);
				} else {
					links++;
					LastLink = pnum;
					*output << directions[links];
					navigation_links.push_back(nav_link{ .paragraph = atol(pnum), .type = nav_type_link, .cost = 0, .monster = {}, .buyable = {}, .prompt = "", .answer = "", .label = label });
				}
				*output << " ";
			} else if (tag.find("<autolink=") != std::string::npos && !last_was_link) {
				int i{0};
				std::string pnum;
				while (p_text[i++] != '=');
				while (p_text[i] != '>') {
					pnum += p_text[i++];
				}
				/* TODO: What is this 'the' madness? Find out! Ancient fix? */
				if (current_player.stamina < 1 || dpp::lowercase(pnum) == "the") {
					navigation_links.push_back(nav_link{ .paragraph = 0, .type = nav_type_respawn, .cost = 0, .monster = {}, .buyable = {}, .prompt = "", .answer = "", .label = "" });
				} else {
					links++;
					LastLink = pnum;
					*output << directions[links];
					if (auto_test) {
						navigation_links.push_back(nav_link{ .paragraph = atol(pnum), .type = nav_type_autolink, .cost = 0, .monster = {}, .buyable = {}, .prompt = "", .answer = "", .label = "" });
					} else {
						navigation_links.push_back(nav_link{ .paragraph = atol(pnum), .type = nav_type_disabled_link, .cost = 0, .monster = {}, .buyable = {}, .prompt = "", .answer = "", .label = "" });
					}
				}
				auto_test = !auto_test; // invert the next autolink...
				*output << " ";
			}
			else {
				if (neat_version.find(">") == std::string::npos && !neat_version.empty() && neat_version[0] != '<') {
					*output << neat_version << " ";
					words++;
				}
			}
		}

	}
	text = output->str();
	delete output;
	if (id && words && safe) {
		if (!current_player.breadcrumb_trail.empty()) {
			if (current_player.breadcrumb_trail[current_player.breadcrumb_trail.size() - 1] == id) {
				co_return;
			}
		}
		current_player.breadcrumb_trail.push_back(id);
		if (current_player.breadcrumb_trail.size() > 10) {
			current_player.breadcrumb_trail.pop_front();
		}
	}
	co_return;
}