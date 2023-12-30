#pragma once

#include <dpp/dpp.h>
#include <ssod/game_player.h>

void game_nav(const dpp::button_click_t& event);
void continue_game(const dpp::interaction_create_t& event, player p);
void game_select(const dpp::select_click_t &event);
void game_input(const dpp::form_submit_t & event);
void send_chat(dpp::snowflake user_id, uint32_t paragraph, const std::string& message, const std::string& type = "chat");