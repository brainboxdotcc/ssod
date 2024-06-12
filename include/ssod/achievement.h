/************************************************************************************
 *
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023,2024 Craig Edwards <brain@ssod.org>
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
#include <vector>
#include <map>
#include <string>
#include "database.h"
#include "game_player.h"
#include "paragraph.h"

void achievement_check(const std::string& event_type, const dpp::interaction_create_t& event, player p, std::map<std::string, json> variables = {}, const paragraph& para = paragraph());
void unlock_achievement(player& p, const db::row& achievement);