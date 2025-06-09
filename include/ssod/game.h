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

#include <dpp/dpp.h>
#include <ssod/game_player.h>
#include <ssod/component_builder.h>

dpp::task<void> game_nav(const dpp::button_click_t& event);
dpp::task<void> continue_game(const dpp::interaction_create_t& event, player p, bool long_response = false);
dpp::task<void> game_select(const dpp::select_click_t &event);
dpp::task<void> game_input(const dpp::form_submit_t & event);
dpp::task<void> send_chat(dpp::snowflake user_id, uint32_t paragraph, const std::string& message, const std::string& type = "chat", uint64_t guild_id = 0);
dpp::task<dpp::emoji> get_emoji(const std::string& name, const std::string& flags);
dpp::task<void> death(player& p, component_builder& cb);
dpp::task<void> do_toasts(player &p, component_builder& cb);