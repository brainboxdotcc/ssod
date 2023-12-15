#include <ssod/game_util.h>
#include <fmt/format.h>
#include <ssod/database.h>
#include <dpp/dpp.h>

dpp::component help_button() {
	return dpp::component()
		.set_type(dpp::cot_button)
		.set_id("player_nav_help")
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
		return fmt::format("Adds {} stamina when used.", modifier_flags.substr(3));
	} else if (modifier_flags.substr(0, 3) == "SK+") {
		return fmt::format("Adds {} skill when used.", modifier_flags.substr(3));
	} else if (modifier_flags.substr(0, 3) == "LK+") {
		return fmt::format("Adds {} luck when used.", modifier_flags.substr(3));
	} else if (modifier_flags.substr(0, 3) == "SN+") {
		return fmt::format("Adds {} sneak when used.", modifier_flags.substr(3));
	} else if (modifier_flags.substr(0, 2) == "W+") {
		return fmt::format("This item adds {} to your weapon score.", modifier_flags.substr(2));
	} else if (modifier_flags.substr(0, 2) == "A+") {
		return fmt::format("This item adds {} to your armour score.", modifier_flags.substr(2));
	} else if (modifier_flags[0] == 'W') {
		return fmt::format("This is a weapon with a rating of {}.",modifier_flags.substr(1));
	} else if (modifier_flags[0] == 'A') {
		return fmt::format("This is armour with a rating of {}.",modifier_flags.substr(1));
	}
	return name;
}
