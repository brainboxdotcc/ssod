#include <ssod/parser.h>

struct comment_tag : public tag {
	comment_tag() { register_tag<comment_tag>(); }
	static constexpr std::string_view tags[]{"<!--"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		while (!paragraph_content.eof() && p_text != "--!>") {
			paragraph_content >> p_text;
		}
	}
};

static comment_tag self_init;
