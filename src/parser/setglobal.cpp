#include <ssod/parser.h>
#include <ssod/database.h>

struct setglobal_tag : public tag {
	setglobal_tag() { register_tag<setglobal_tag>(); }
	static constexpr std::string_view tags[]{"<setglobal"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;
		p_text = dpp::lowercase(remove_last_char(p_text));
		db::query("REPLACE INTO game_global_flags (flag) VALUES(?)", {p_text});
	}
};

static setglobal_tag self_init;
