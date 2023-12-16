#include <ssod/parser.h>
#include <ssod/game_dice.h>

struct twod6_tag : public tag {
	twod6_tag() { register_tag<twod6_tag>(); }
	static constexpr std::string_view tags[]{"<2d6>"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		p.g_dice = dice() + dice();
	}
};

static twod6_tag self_init;
