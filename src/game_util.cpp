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
#include <ssod/game_util.h>
#include <fmt/format.h>
#include <ssod/database.h>
#include <dpp/dpp.h>
#include <ssod/aes.h>

dpp::component help_button() {
	return dpp::component()
		.set_type(dpp::cot_button)
		.set_id(security::encrypt("player_nav_help"))
		.set_label("Get Help")
		.set_url("https://discord.gg/brainbox")
		.set_style(dpp::cos_link);		
}

std::string describe_item(const std::string& modifier_flags, const std::string& name)
{
	auto res = db::query("SELECT idesc FROM game_item_descs WHERE name = ?", {name});
	std::string rv{res.size() ? res[0].at("idesc") : ""};
	if (!rv.empty()) {
		return rv;
	}
	
	if (modifier_flags.empty()) {
		return name;
	}
	if (modifier_flags.substr(0, 3) == "ST+") {
		return fmt::format("Adds {} stamina when used", modifier_flags.substr(3));
	} else if (modifier_flags.substr(0, 3) == "SK+") {
		return fmt::format("Adds {} skill when used", modifier_flags.substr(3));
	} else if (modifier_flags.substr(0, 3) == "LK+") {
		return fmt::format("Adds {} luck when used", modifier_flags.substr(3));
	} else if (modifier_flags.substr(0, 3) == "SN+") {
		return fmt::format("Adds {} sneak when used", modifier_flags.substr(3));
	} else if (modifier_flags.substr(0, 2) == "W+") {
		return fmt::format("Adds {} to your weapon score", modifier_flags.substr(2));
	} else if (modifier_flags.substr(0, 2) == "A+") {
		return fmt::format("Adds {} to your armour score", modifier_flags.substr(2));
	} else if (modifier_flags[0] == 'W') {
		return fmt::format("Weapon, rating {}",modifier_flags.substr(1));
	} else if (modifier_flags[0] == 'A') {
		return fmt::format("Armour, rating {}",modifier_flags.substr(1));
	}
	return name;
}

void premium_required(const dpp::interaction_create_t& event) {
	event.reply(
		dpp::message("## Premium Required\n\nYou need [Seven Spells Premium](https://premium.ssod.org) to use this feature! Subscriptions are just £3 a month and give access to additional areas, automatic loot drops, and more!")
		.set_flags(dpp::m_ephemeral)
		.add_component(
			dpp::component().add_component(
				dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_premium_cta"))
				.set_label("Get Premium")
				.set_url("https://premium.ssod.org/?user=" + event.command.usr.id.str())
				.set_style(dpp::cos_link)
			)
		)
	);
}