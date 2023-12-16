#include <ssod/parser.h>
#include <ssod/database.h>

struct eat_tag : public tag {
	eat_tag() { register_tag<eat_tag>(); }
	static constexpr std::string_view tags[]{"<eat>"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		if (!p.didntmove) {
			current_player.eat_ration();
		}
	}
};

static eat_tag self_init;
