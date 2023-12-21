#include <ssod/parser.h>
#include <ssod/database.h>

struct test_tag : public tag {
	test_tag() { register_tag<test_tag>(); }
	static constexpr std::string_view tags[]{"<test"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// test score tag
		paragraph_content >> p_text;
		p_text = dpp::lowercase(p_text);
		p.words++;

		if (p_text.find("luck>") != std::string::npos) {
			output << " Test your __**luck**__. ";
			p.auto_test = current_player.test_luck();
		} else if (p_text.find("stamina>") != std::string::npos) {
			output << " Test your __**stamina**__. ";
			p.auto_test = current_player.test_stamina();
		} else if (p_text.find("skill>") != std::string::npos) {
			output << " Test your __**skill**__. ";
			p.auto_test = current_player.test_skill();
		} else if (p_text.find("speed>") != std::string::npos) {
			output << " Test your __**speed**__. ";
			p.auto_test = current_player.test_speed();
		} else if (p_text.find("exp>") != std::string::npos) {
			output << " Test your __**experience**__. ";
			p.auto_test = current_player.test_experience();
		}
	}
};

static test_tag self_init;
