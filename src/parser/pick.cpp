#include <ssod/parser.h>

struct pick_tag : public tag {
	pick_tag() { register_tag<pick_tag>(); }
	static constexpr std::string_view tags[]{"<pick"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// pick up free items (one-choice)
		paragraph_content >> p_text;
		extract_to_quote(p_text, paragraph_content, '"');
		std::string ItemName = extract_value(p_text);
		paragraph_content >> p_text;
		std::string ItemVal = extract_value(p_text);
		// TODO: Action Button
		//sprintf(New,"action=pick&guid=%s&item=%s&val=%s",formData[1],AddEscapes(ItemName),ItemVal);
		if (not_got_yet(p.id, "PICKED", current_player.gotfrom)) {
			output << "\n **" << ItemName << "** ";
			output << directions[++p.links] << "\n";
			p.navigation_links.push_back(nav_link{ .paragraph = p.id, .type = nav_type_pick_one, .cost = 0, .monster = {}, .buyable = { .name = ItemName, .flags = ItemVal } });
		} else {
			output << "\n **" << ItemName << "** ";
		}
	}
};

static pick_tag self_init;
