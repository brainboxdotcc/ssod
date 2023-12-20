#pragma once
#include <string>
#include <vector>
#include <dpp/dpp.h>
#include <ssod/game_player.h>
#include <dpp/unicode_emoji.h>

inline const char* directions[10] = {
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
};

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

struct nav_link {
	long paragraph;
	nav_link_type type;
	long cost;
	enemy monster;
	item buyable;
};

struct paragraph {
	uint32_t id{};
	std::string text;
	std::vector<nav_link> navigation_links;
	std::vector<item> dropped_items;
	bool combat_disabled{};
	bool magic_disabled{};
	bool theft_disabled{};
	bool chat_disabled{};

	size_t links{0}, words{0};
	std::string tag;
	bool last_was_link{false};
	bool display{true};
	long current_fragment{0};	
	bool auto_test{false}, didntmove{false};
	int g_dice{0};

	paragraph() = default;
	~paragraph() = default;
	paragraph(uint32_t paragraph_id, player& current, dpp::snowflake user_id);

	static bool valid_next(long Current, long Next);

private:
	void parse(player& current_player, dpp::snowflake user_id);
};

std::string extract_value(const std::string& p_text);

long extract_value_number(const std::string& p_text);

bool global_set(const std::string& flag);

bool not_got_yet(uint32_t paragraph, const std::string& item, const std::string& gotfrom);

void extract_to_quote(std::string& p_text, std::stringstream& content, char end = '>');

std::string remove_last_char(const std::string& s);
