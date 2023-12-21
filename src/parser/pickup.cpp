#include <ssod/parser.h>

struct pickup_tag : public tag {
	pickup_tag() { register_tag<pickup_tag>(); }
	static constexpr std::string_view tags[]{"<pickup"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string item_name, item_flags, flags;

		paragraph_content >> p_text;

		if (dpp::lowercase(p_text) == "scroll>") {
			if (not_got_yet(p.id, "SCROLL", current_player.gotfrom)) {
				current_player.scrolls++;
				current_player.add_flag("SCROLL", p.id);
			}
			return;
		}

		if (dpp::lowercase(p_text) == "gold") {
			paragraph_content >> p_text;
			p_text = remove_last_char(p_text);
			current_player.add_gold(atoi(p_text.c_str()));
			return;
		}

		if (dpp::lowercase(p_text) == "silver") {
			paragraph_content >> p_text;
			p_text = remove_last_char(p_text);
			current_player.add_silver(atoi(p_text.c_str()));
			return;
		}

		item_name = p_text;
		item_flags = "[[none]]";

		while (p_text.length() && *p_text.rbegin() != '>') {
			paragraph_content >> p_text;
			if (p_text.length() && p_text[0] != '[') {
				item_name += " " + p_text;
			} else {
				item_flags = p_text;
				item_flags = remove_last_char(item_flags);
			}
		}
		if (item_name.length() && *item_name.rbegin() == '>') {
			item_name = remove_last_char(item_name);
		}
		// strip the [ and ] from the item flags...
		for (size_t i = 1; i < item_flags.length() - 1; ++i) {
			flags += item_flags[i];
		}

		if (!not_got_yet(p.id, item_name, current_player.gotfrom)) {
			// crafty player trying to get the same item twice! Not good if its unique!
			return;
		}
		current_player.add_flag(item_name, p.id);
		if (flags == "SPELL") {
			current_player.spells.push_back(item{ .name = item_name, .flags = flags });
		} else if (flags == "HERB") {
			current_player.herbs.push_back(item{ .name = item_name, .flags = flags });
		} else {
			current_player.possessions.push_back(item{ .name = item_name, .flags = flags });
		}
		return;
	}
};

static pickup_tag self_init;
