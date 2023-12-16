#include <ssod/parser.h>

struct br_tag : public tag {
	br_tag() { register_tag<br_tag>(); }
	static constexpr std::string_view tags[]{"<br>"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		output << "\n";
	}
};

static br_tag self_init;
