#include <ssod/parser.h>

struct else_tag : public tag {
	else_tag() { register_tag<else_tag>(); }
	static constexpr std::string_view tags[]{"<else>"};
	static constexpr bool overrides_display{true};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// simply invert the display flag for anything inside an
		// <else> tag...
		p.display = !p.display;
	}
};

static else_tag self_init;
