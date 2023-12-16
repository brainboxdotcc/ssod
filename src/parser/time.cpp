#include <ssod/parser.h>
#include <ssod/database.h>

struct time_tag : public tag {
	time_tag() { register_tag<time_tag>(); }
	static constexpr std::string_view tags[]{"<time>"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		if (!p.didntmove) {
			current_player.eat_ration();
		}
	}
};

static time_tag self_init;
