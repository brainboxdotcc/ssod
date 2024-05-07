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
#include <cstdint>
#include <dpp/dpp.h>

struct sale_info {
	std::string flags;
	long value{1};
	bool sellable{false};
	bool quest_item{false};
};

std::string describe_item(const std::string& modifier_flags, const std::string& name, const dpp::interaction_create_t& event, bool ansi = false, size_t max_desc_len = 250);
dpp::component help_button();
void premium_required(const dpp::interaction_create_t& event);
sale_info get_sale_info(const std::string& name);
std::string human_readable_spell_name(const std::string& spell, const dpp::interaction_create_t& event);
std::string human_readable_herb_name(const std::string& herb, const dpp::interaction_create_t& event);