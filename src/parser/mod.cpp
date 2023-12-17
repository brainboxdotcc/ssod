#include <ssod/ssod.h>
#include <ssod/parser.h>

struct mod_tag : public tag {
	mod_tag() { register_tag<mod_tag>(); }
	static constexpr std::string_view tags[]{"<mod"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string mod;
		paragraph_content >> p_text;
		paragraph_content >> mod;
		mod = remove_last_char(mod); // remove '>'
		long modifier = atol(mod);
		std::string flag = "MOD" + p_text + mod;

		const std::map<std::string, std::pair<std::string, long*>> modifier_list = {
			{"stm", {"stamina", &current_player.stamina}},
			{"skl", {"skill", &current_player.skill}},
			{"luck", {"luck", &current_player.luck}},
			{"exp", {"experience", &current_player.experience}},
			{"arm", {"armour", &current_player.armour.rating}},
			{"wpn", {"weapon", &current_player.weapon.rating}},
			{"spd", {"speed", &current_player.speed}},
		};
		auto m = modifier_list.find(p_text);
		if (m != modifier_list.end()) {
			// No output if the player's been here before
			if (not_got_yet(p.id, flag, current_player.gotfrom)) {
				current_player.gotfrom += " [" + flag + std::to_string(p.id) + "]";
			} else {
				output << "***Make no changes to your " << m->first << "*** ";
				return;
			}
			output << " ***" << (modifier < 1 ? "Subtract " : "Add ") << abs(modifier) << " " << m->second.first << "*** ";
			*(m->second.second) += modifier;
		}
	}
};

static mod_tag self_init;
