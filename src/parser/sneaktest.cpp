#include <ssod/parser.h>

struct sneaktest_tag : public tag {
	sneaktest_tag() { register_tag<sneaktest_tag>(); }
	static constexpr std::string_view tags[]{"<sneaktest"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;

		extract_to_quote(p_text, paragraph_content, '"');

		std::string monster_name = extract_value(p_text);
		paragraph_content >> p_text;
		long monster_sneak = extract_value_number(p_text);
		output << "\n***" << monster_name << "** *Sneak " << monster_sneak << "*,";
		p.auto_test = current_player.sneak_test(monster_sneak);
		output << (p.auto_test ? " **PASSED**!\n" : " **FAILED**!\n");
		p.words++;
	}
};

static sneaktest_tag self_init;
