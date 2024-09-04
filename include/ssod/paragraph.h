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
#include <vector>
#include <dpp/dpp.h>
#include <ssod/game_player.h>
#include <dpp/unicode_emoji.h>
#include <sstream>

/**
 * Emojis used for travel buttons
 */
inline const char* directions[] = {
	dpp::unicode_emoji::zero,
	dpp::unicode_emoji::one,
	dpp::unicode_emoji::two,
	dpp::unicode_emoji::three,
	dpp::unicode_emoji::four,
	dpp::unicode_emoji::five,
	dpp::unicode_emoji::six,
	dpp::unicode_emoji::seven,
	dpp::unicode_emoji::eight,
	dpp::unicode_emoji::nine,
	dpp::unicode_emoji::regional_indicator_a,
	dpp::unicode_emoji::regional_indicator_b,
	dpp::unicode_emoji::regional_indicator_c,
	dpp::unicode_emoji::regional_indicator_d,
	dpp::unicode_emoji::regional_indicator_e,
	dpp::unicode_emoji::regional_indicator_f,
	dpp::unicode_emoji::regional_indicator_g,
	dpp::unicode_emoji::regional_indicator_h,
	dpp::unicode_emoji::regional_indicator_i,
	dpp::unicode_emoji::regional_indicator_j,
	dpp::unicode_emoji::regional_indicator_k,
	dpp::unicode_emoji::regional_indicator_l,
	dpp::unicode_emoji::regional_indicator_m,
	dpp::unicode_emoji::regional_indicator_n,
	dpp::unicode_emoji::regional_indicator_o,
	dpp::unicode_emoji::regional_indicator_p,
	dpp::unicode_emoji::regional_indicator_q,
	dpp::unicode_emoji::regional_indicator_r,
	dpp::unicode_emoji::regional_indicator_s,
	dpp::unicode_emoji::regional_indicator_t,
	dpp::unicode_emoji::regional_indicator_u,
	dpp::unicode_emoji::regional_indicator_v,
	dpp::unicode_emoji::regional_indicator_w,
	dpp::unicode_emoji::regional_indicator_x,
	dpp::unicode_emoji::regional_indicator_y,
	dpp::unicode_emoji::regional_indicator_z,
};

/**
 * Nav link button types
 */
enum nav_link_type {
	nav_type_disabled_link,
	nav_type_link,
	nav_type_paylink,
	nav_type_autolink,
	nav_type_modal,
	nav_type_shop,
	nav_type_combat,
	nav_type_bank,
	nav_type_respawn,
	nav_type_pick_one,
};

/**
 * A navigation link
 */
struct nav_link {
	long paragraph;
	nav_link_type type;
	long cost;
	enemy monster;
	item buyable;
	std::string prompt;
	std::string answer;
	std::string label;
};

struct paragraph {
	/**
	 * Paragraph ID for accessing row in database
	 */
	uint32_t id{};

	/**
	 * Temporary text value for last token read
	 */
	std::string text;

	/**
	 * Secure ID, used for display of location to user
	 */
	std::string secure_id;

	/**
	 * List of possible links out of this paragraph,
	 * to other related paragraphs or actions
	 */
	std::vector<nav_link> navigation_links;

	/**
	 * Stacks of items dropped to the ground here that
	 * the player can pick up.
	 */
	std::vector<stacked_item> dropped_items;

	/**
	 * Output stream, contains content from database
	 */
	std::stringstream* output{nullptr};

	/**
	 * True if players cannot PVP here
	 */
	bool combat_disabled{};

	/**
	 * True if players cannot cast passive spells here
	 */
	bool magic_disabled{};

	/**
	 * True if players cannot steal from each other here
	 */
	bool theft_disabled{};

	/**
	 * True if players cannot chat here
	 */
	bool chat_disabled{};

	/**
	 * True if this location is safe, determined by if the location
	 * does not adjust stats, and has no PVE combat.
	 */
	bool safe{true};

	/**
	 * True if this location has a shop or trader here, which means
	 * we should allow the player to sell items.
	 */
	bool trader{false};

	/**
	 * True if the player has a disease
	 */
	bool sick{false};

	/**
	 * Number of outbound links found in the content - this may be different to
	 * navigation_links.size() as not all navigation links are outbound
	 * links that lead elsewhere.
	 */
	size_t links{0};

	/**
	 * Number of printable non-code words in the paragraph
	 */
	size_t words{0};

	/**
	 * Current tag name whilst parsing
	 */
	std::string tag;

	/**
	 * True if the last element found in the paragraph text was a link
	 */
	bool last_was_link{false};

	/**
	 * A stack of display states, while the top display state on the stack
	 * is true, we output text and action tags, else we discard them.
	 */
	std::vector<bool> display;

	/**
	 * Current fragment. Where a paragraph has combat encounters, each section
	 * before that encounter's combat tag to the next is known as a fragment.
	 * We only display the current fragment, e.g. up to and including the current
	 * combat encounter the player is engaged in. When the beat that enemy, the
	 * fragment count is incremented, until they are on the last fragment, where
	 * potentially there are active ountbound links to leave the location.
	 */
	long current_fragment{0};

	/**
	 * The result of the last <TEST LUCK> etc tag in the location
	 */
	bool auto_test{false};

	/**
	 * True if the users last interaction with the game didn't result in them moving
	 * to a new location
	 */
	bool didntmove{false};

	/**
	 * Pointer to current player, used by js interpreters
	 */
	player* cur_player{nullptr};

	/**
	 * Parent paragraph ID for macros, zero otherwise
	 */
	uint32_t parent{};

	/**
	 * Default constructor
	 */
	paragraph() = default;

	/**
	 * Default destructor
	 */
	~paragraph() = default;

	/**
	 * Given a current paragraph id, return true if the next id is a valid move,
	 * or false if it isn't. Used as a double check against cheating.
	 * @param current current id
	 * @param next  next id
	 * @return true if valid move
	 */
	static dpp::task<bool> valid_next(long current, long next);

	/**
	 * @brief Factory function to create a new paragraph instance with a paragraph id, player and user id
	 * @param paragraph_id Paragraph ID to construct and parse
	 * @param current Current player
	 * @param user_id Discord user id
	 * @param parent_id parent paragraph ID for macros
	 * @return awaitable which will return paragraph when fulfilled
	 */
	static dpp::task<paragraph> create(uint32_t paragraph_id, player& current, dpp::snowflake user_id, uint32_t parent_id = 0);

	/**
	 * @brief Factory function to create a new paragraph instance with existing content and a player
	 * @param data String containing existing content to parse
	 * @param current Current player
	 * @return awaitable which will return paragraph when fulfilled
	 */
	static dpp::task<paragraph> create(const std::string& data, player& current);

protected:
	/**
	 * Construct a paragraph with existing data.
	 * @note Does not parse the content, this is done asynchronously via the parse method.
	 * @param data Paragraph content
	 * @param current Current player
	 */
	paragraph(const std::string& data, player& current);

	/**
	 * Construct a paragraph from paragraph id
	 * @note Does not parse the content, this is done asynchronously via the parse method.
	 * @param paragraph_id Paragraph id in database
	 * @param current Current player
	 * @param user_id Discord user id
	 * @param parent_id parent paragraph ID for macros
	 */
	paragraph(uint32_t paragraph_id, player& current, dpp::snowflake user_id, uint32_t parent_id = 0);

	/**
	 * Parse paragraph content, this may take some time to complete as it has to interpret the content
	 * and retrieve it from the database.
	 * @param current_player Current active player
	 * @param user_id Discord user id
	 * @return awaitable
	 */
	dpp::task<void> parse(player& current_player, dpp::snowflake user_id);
};

/**
 * Extract the next string value from a key/value list, e.g.
 * if the string contains 'NAME="Big Monster"' this will return
 * Big Monster.
 * @param p_text text to extract from
 * @return extracted string
 */
std::string extract_value(const std::string& p_text);

/**
 * Extract the next numeric value from a key/value list, e.g.
 * if the string contains 'A="1"' this will return 1.
 * @param p_text text to extract from
 * @return extracted number
 */
long extract_value_number(const std::string& p_text);

/**
 * Returns true if a global flag is set
 * @param flag flag name
 * @return awaitable true if set
 */
dpp::task<bool> global_set(const std::string& flag);

/**
 * Returns true if a timed flag is set
 * @param event user's event
 * @param flag flag name
 * @return awaitable true if set
 */
dpp::task<bool> timed_set(dpp::interaction_create_t& event, const std::string& flag);

/**
 * Given some text, extract all text up to but not including the end character
 * @param p_text text to extract into
 * @param content stream to extract from
 * @param end end character to stop at
 */
void extract_to_quote(std::string& p_text, std::stringstream& content, char end = '>');

/**
 * Removes the last character from a string
 * @param s string to remove from
 * @return modified string
 */
std::string remove_last_char(const std::string& s);
