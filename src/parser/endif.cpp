#include <ssod/parser.h>

struct endif_tag : public tag {
	endif_tag() { register_tag<endif_tag>(); }
	static constexpr std::string_view tags[]{"<endif>"};
	static constexpr bool overrides_display{true};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		p.display = true;
	}
};

static endif_tag self_init;
