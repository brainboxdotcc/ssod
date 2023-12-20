#pragma once
#include <dpp/dpp.h>
#include <ssod/game_player.h>

bool combat_nav(const dpp::button_click_t& event, player p, const std::vector<std::string>& parts);

void continue_combat(const dpp::interaction_create_t& event, player p);