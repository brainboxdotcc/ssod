#include <ssod/parser.h>

struct set_tag : public tag {
	set_tag() { register_tag<set_tag>(); }
	static constexpr std::string_view tags[]{"<set"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// set a state-flag
		paragraph_content >> p_text;
		p_text = dpp::lowercase(remove_last_char(p_text));
		current_player.add_flag("gamestate_" + p_text, p.id);
	}
};

static set_tag self_init;
