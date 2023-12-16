#include <ssod/parser.h>
#include <ssod/database.h>

struct unsetglobal_tag : public tag {
	unsetglobal_tag() { register_tag<unsetglobal_tag>(); }
	static constexpr std::string_view tags[]{"<unsetglobal"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;
		p_text = dpp::lowercase(remove_last_char(p_text));
		db::query("DELETE FROM game_global_flags WHERE flag = ?", {p_text});
	}
};

static unsetglobal_tag self_init;
