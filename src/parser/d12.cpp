#include <ssod/parser.h>
#include <ssod/game_dice.h>

struct d12_tag : public tag {
	d12_tag() { register_tag<d12_tag>(); }
	static constexpr std::string_view tags[]{"<d12>"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		p.g_dice = d12();
	}
};

static d12_tag self_init;
