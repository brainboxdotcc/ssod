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

struct paragraph {
	uint32_t id{};
	std::string text;
	std::vector<uint32_t> navigation_links;
	std::vector<item> dropped_items;
	bool combat_disabled{};
	bool magic_disabled{};
	bool theft_disabled{};
	bool chat_disabled{};

	paragraph() = default;
	~paragraph() = default;
	paragraph(uint32_t paragraph_id, player current, dpp::snowflake user_id);

private:
	void parse(player current_player, dpp::snowflake user_id);
};
