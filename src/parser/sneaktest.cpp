#include <ssod/parser.h>

struct sneaktest_tag : public tag {
	sneaktest_tag() { register_tag<sneaktest_tag>(); }
	static constexpr std::string_view tags[]{"<sneaktest"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;

		extract_to_quote(p_text, paragraph_content);

		std::string MonsterName = extract_value(p_text);
		paragraph_content >> p_text;
		long MonsterSneak = extract_value_number(p_text);
		output << "\n***" << MonsterName << "** *Sneak " << MonsterSneak << "*,";
		p.auto_test = current_player.sneak_test(MonsterSneak);
		output << (p.auto_test ? " **PASSED**!\n" : " **FAILED**!\n");
	}
};

static sneaktest_tag self_init;
