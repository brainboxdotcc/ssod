#include <ssod/parser.h>

struct input_tag : public tag {
	input_tag() { register_tag<input_tag>(); }
	static constexpr std::string_view tags[]{"<input"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// <input prompt="prompt" location="loc_id" value="correct_answer">
		// TODO: Modal Dialog!
		p.links++;
		paragraph_content >> p_text;
		extract_to_quote(p_text, paragraph_content, '"');
		std::string Prompt = extract_value(p_text);
		paragraph_content >> p_text;
		std::string Para = extract_value(p_text);
		paragraph_content >> p_text;
		std::string Correct = extract_value(p_text);
		output << "\n\n";
		//output << "<b>" << Prompt << "</b><br><form action='" << me << "'><input type='hidden' name='action' value='riddle'><input type='hidden' name='guid' value='" << formData[1] << "'><input type='hidden' name='keycode' value='" << Key << "'><input type='text' name='q' value='""'><input type='hidden' name='p' value='" << Para << "'><input type='submit' value='Answer'></form>" << CR << CR;
		output << "TODO: Input Dialog\n";
		p.words++;
	}
};

static input_tag self_init;
