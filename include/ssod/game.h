#pragma once

#include <dpp/dpp.h>
#include <ssod/game_player.h>

void game_nav(const dpp::button_click_t& event);
void continue_game(const dpp::interaction_create_t& event, player p);
void game_select(const dpp::select_click_t &event);