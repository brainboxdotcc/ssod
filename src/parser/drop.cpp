#include <ssod/parser.h>

struct drop_tag : public tag {
	drop_tag() { register_tag<drop_tag>(); }
	static constexpr std::string_view tags[]{"<drop"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;
		std::string i{p_text};
		while (p_text.length() && *p_text.rbegin() != '>') {
			paragraph_content >> p_text;
			i += " " + p_text;
		}
		i = remove_last_char(i); // remove '>'
		current_player.drop_possession(item{ .name = i, .flags = "" });
		current_player.drop_spell(item{ .name = i, .flags = "" });
		current_player.drop_herb(item{ .name = i, .flags = "" });
	}
};

static drop_tag self_init;
