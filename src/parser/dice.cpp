#include <ssod/parser.h>
#include <ssod/game_dice.h>

struct dice_tag : public tag {
	dice_tag() { register_tag<dice_tag>(); }
	static constexpr std::string_view tags[]{"<dice>"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		p.g_dice = dice();
	}
};

static dice_tag self_init;
