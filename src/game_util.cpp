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
#include <ssod/ssod.h>

dpp::component help_button() {
	return dpp::component()
		.set_type(dpp::cot_button)
		.set_id(security::encrypt("player_nav_help"))
		.set_label("Get Help")
		.set_url("https://discord.gg/brainbox")
		.set_style(dpp::cos_link);		
}

sale_info get_sale_info(const std::string& name) {
	auto res = db::query("SELECT * FROM game_item_descs WHERE name = ?", {name});
	if (res.empty()) {
		return sale_info{};
	}
	return sale_info{
		.flags = res[0].at("flags"),
		.value = atol(res[0].at("value")),
		.sellable = res[0].at("sellable") == "1",
		.quest_item = res[0].at("quest_item") == "1",
	};
}

std::string ellipsis(const std::string& in, size_t max_len) {
	if (in.length() > max_len) {
		return in.substr(0, max_len) + "…";
	}
	return in;
}

std::string describe_item(const std::string& modifier_flags, const std::string& name, bool ansi, size_t max_desc_len) {
	auto res = db::query("SELECT idesc FROM game_item_descs WHERE name = ?", {name});
	std::string rv{ellipsis(res.size() ? res[0].at("idesc") : name, max_desc_len)};

	if (modifier_flags.substr(0, 3) == "ST+") {
		return fmt::format(fmt::runtime(ansi ? "\033[2;36mStamina\033[0m \033[2;34m+{}\033[0m: {}" : "Stamina **+{}**: {}"), modifier_flags.substr(3), rv);
	} else if (modifier_flags.substr(0, 3) == "SK+") {
		return fmt::format(fmt::runtime(ansi ? "\033[2;36mSkill\033[0m \033[2;34m+{}\033[0m: {}" : "Skill **+{}**: {}"), modifier_flags.substr(3), rv);
	} else if (modifier_flags.substr(0, 3) == "LK+") {
		return fmt::format(fmt::runtime(ansi ? "\033[2;36mLuck\033[0m \033[2;34m+{}\033[0m: {}" : "Luck **+{}**: {}"), modifier_flags.substr(3), rv);
	} else if (modifier_flags.substr(0, 3) == "SN+") {
		return fmt::format(fmt::runtime(ansi ? "\033[2;36mSneak\033[0m \033[2;34m+{}\033[0m: {}" : "Sneak **+{}**: {}"), modifier_flags.substr(3), rv);
	} else if (modifier_flags.substr(0, 2) == "W+") {
		return fmt::format(fmt::runtime(ansi ? "\033[2;36mWeapon\033[0m \033[2;34m+{}\033[0m: {}" : "Weapon **+{}**: {}"), modifier_flags.substr(2), rv);
	} else if (modifier_flags.substr(0, 2) == "A+") {
		return fmt::format(fmt::runtime(ansi ? "\033[2;36mArmour\033[0m \033[2;34m+{}\033[0m: {}" : "Armour **+{}**: {}"), modifier_flags.substr(2), rv);
	} else if (!modifier_flags.empty() && modifier_flags[0] == 'W') {
		return fmt::format(fmt::runtime(ansi ? "\033[2;36mWeapon\033[0m \033[2;34m{}\033[0m: {}" : "Weapon **{}**: {}"),modifier_flags.substr(1), rv);
	} else if (!modifier_flags.empty() && modifier_flags[0] == 'A') {
		return fmt::format(fmt::runtime(ansi ? "\033[2;36mArmour\033[0m \033[2;34m{}\033[0m: {}" : "Armour **{}**: {}"),modifier_flags.substr(1), rv);
	}
	return rv;
}

void premium_required(const dpp::interaction_create_t& event) {
	event.reply(
		dpp::message(_("PREMIUMUPSELL", event))
		.set_flags(dpp::m_ephemeral)
		.add_component(
			dpp::component().add_component(
				dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_premium_cta"))
				.set_label(_("GETPREMIUM", event))
				.set_url("https://premium.ssod.org/?user=" + event.command.usr.id.str())
				.set_style(dpp::cos_link)
			)
		)
	);
}

static const std::map<std::string, std::string> human_readable_spells = {
	{ "fire", "Fire" },
	{ "water", "Water" },
	{ "light", "Light" },
	{ "fly", "Fly" },
	{ "strength", "Strength" },
	{ "xray", "X-Ray" },
	{ "bolt", "Bolt" },
	{ "fasthands", "Fast Hands" },
	{ "thunderbolt", "Thunderbolt" },
	{ "steal", "Steal" },
	{ "shield", "Shield" },
	{ "jump", "Jump" },
	{ "open", "Open" },
	{ "spot", "Spot" },
	{ "sneak", "Sneak" },
	{ "esp", "E.S.P." },
	{ "run", "Run" },
	{ "invisible", "Invisible" },
	{ "shrink", "Shrink" },
	{ "grow", "Grow" },
	{ "air", "Air" },
	{ "animalcommunication", "Animal Communication" },
	{ "weaponskill", "Weapon Skill" },
	{ "healing", "Healing" },
	{ "woodsmanship", "Woodsmanship" },
	{ "nightvision", "Night Vision" },
	{ "heateyes", "Heat Eyes" },
	{ "decipher", "Decipher" },
	{ "detect", "Detect" },
	{ "tracking", "Tracking" },
	{ "espsurge", "E.S.P. Surge" },
	{ "afterimage", "After Image" },
	{ "psychism", "Psychism" },
	{ "spiritwalk", "Spirit Walk" },
	{ "growweapon", "Grow Weapon" },
	{ "teleport", "Teleport" },
	{ "vortex", "Vortex" },
};

static const std::map<std::string, std::string> human_readable_herbs = {
	{ "hartleaf", "Hartleaf" },
	{ "elfbane", "Elfbane" },
	{ "monkgrass", "Monkgrass" },
	{ "fireseeds", "Fireseeds" },
	{ "woodweed", "Woodweed" },
	{ "blidvines", "Blidvines" },
	{ "stickwart", "Stickwart" },
	{ "spikegrass", "Spikegrass" },
	{ "hallucinogen", "Hallucinogen" },
	{ "wizardsivy", "Wizard's Ivy" },
	{ "orcweed", "Orcweed" },
	{ "arrowroot", "Arrowroot" },
	{ "windherb", "Windherb" },
};


std::string human_readable_spell_name(const std::string& spell) {
	auto s = human_readable_spells.find(spell);
	return s == human_readable_spells.end() ? spell : s->second;
}

std::string human_readable_herb_name(const std::string& herb) {
	auto h = human_readable_herbs.find(herb);
	return h == human_readable_herbs.end() ? herb : h->second;
}
