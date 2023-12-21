#include <ssod/parser.h>

struct b_tag : public tag {
	b_tag() { register_tag<b_tag>(); }
	static constexpr std::string_view tags[]{"<b>", "</b>"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		output << "**";
		p.words++;
	}
};

static b_tag self_init;
