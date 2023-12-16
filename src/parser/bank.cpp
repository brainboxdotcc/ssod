#include <ssod/parser.h>

struct bank_tag : public tag {
	bank_tag() { register_tag<bank_tag>(); }
	static constexpr std::string_view tags[]{"<bank>"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		p.links++;
		p.navigation_links.push_back(nav_link{ .paragraph = p.id, .type = nav_type_bank, .cost = 0, .monster = {}, .buyable = {} });
	}
};

static bank_tag self_init;
