#include <ssod/parser.h>

bool comparison(std::string condition, long C1, const std::string& C2, int g_dice) {
	long C = C2 == "dice" ? g_dice : atol(C2.c_str());
	condition = dpp::lowercase(condition);
	if (condition == "eq" && C1 == C) {
		return true;
	} else if (condition == "gt" && C1 > C) {
		return true;
	} else if (condition == "lt" && C1 < C) {
		return true;
	} else if (condition == "ne" && C1 != C) {
		return true;
	} else {
		return false;
	}
}

struct if_tag : public tag {
	if_tag() { register_tag<if_tag>(); }
	static constexpr bool overrides_display{true};
	static constexpr std::string_view tags[]{"<if"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string condition;
		paragraph_content >> p_text;
		// -------------------------------------------------------
		// <if item multi-word-item-name>
		// -------------------------------------------------------
		if (dpp::lowercase(p_text) == "item") {
			paragraph_content >> p_text;
			extract_to_quote(p_text, paragraph_content, '>');
			p_text = remove_last_char(p_text);
			p.display = current_player.has_herb(p_text) || current_player.has_spell(p_text) || current_player.has_possession(p_text);
			return;
		} else if (dpp::lowercase(p_text) == "flag") {
			paragraph_content >> p_text;
			p_text = remove_last_char(p_text);
			std::string flag = " gamestate_" + p_text;
			p.display = (current_player.gotfrom.find(flag) != std::string::npos || global_set(p_text));
			return;
		} else if (dpp::lowercase(p_text) == "!flag") {
			paragraph_content >> p_text;
			p_text = remove_last_char(p_text);
			std::string flag = " gamestate_" + p_text;
			p.display = (current_player.gotfrom.find(flag) == std::string::npos && !global_set(p_text));
			return;
		}
		// -------------------------------------------------------
		// <if scorename gt|lt|eq value>
		// -------------------------------------------------------
		std::string scorename = dpp::lowercase(p_text);
		const std::map<std::string, long> scorename_map = {
			{ "exp", current_player.experience },
			{ "dice", p.g_dice },
			{ "stm", current_player.stamina },
			{ "skl", current_player.skill },
			{ "arm", current_player.armour.rating },
			{ "wpn", current_player.weapon.rating },
			{ "day", current_player.days },
			{ "spd", current_player.speed },
			{ "luck", current_player.luck },
		};
		auto check = scorename_map.find(scorename);
		if (check != scorename_map.end()) {
			paragraph_content >> condition;
			paragraph_content >> p_text;
			p.display = comparison(condition, check->second, p_text, p.g_dice);
		} else if (dpp::lowercase(p_text) == "race") {
			// ------------------------------------------------------
			// <if race x>
			// ------------------------------------------------------
			// if false, nothing displayed until an <endif> is reached.
			paragraph_content >> p_text;
			p.display =
				(dpp::lowercase(p_text) == "human>" && (current_player.race == race_human || current_player.race == race_barbarian))
					||
				(dpp::lowercase(p_text) == "orc>" && (current_player.race == race_orc || current_player.race == race_goblin))
					||
				(dpp::lowercase(p_text) == "elf>" && (current_player.race == race_elf || current_player.race == race_dark_elf))
					||
				(dpp::lowercase(p_text) == "dwarf>" && current_player.race == race_dwarf)
					||
				(dpp::lowercase(p_text) == "lesserorc>" && current_player.race == race_lesser_orc);
			return;
		} else if (dpp::lowercase(p_text) == "prof") {
			// ------------------------------------------------------
			// <if prof x>
			// ------------------------------------------------------
			// if false, nothing displayed until an <endif> is reached.
			paragraph_content >> p_text;
			p.display = 
				(dpp::lowercase(p_text) == "warrior>" && (current_player.profession == prof_warrior || current_player.profession == prof_mercenary))
					||
				(dpp::lowercase(p_text) == "wizard>" && current_player.profession == prof_wizard)
					||
				(dpp::lowercase(p_text) == "thief>" && (current_player.profession == prof_thief || current_player.profession == prof_assassin))
					||
				(dpp::lowercase(p_text) == "woodsman>" && current_player.profession == prof_woodsman);
			return;
		}
	}
};

static if_tag self_init;
