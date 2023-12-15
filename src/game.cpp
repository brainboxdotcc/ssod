#include <dpp/dpp.h>
#include <string>
#include <ssod/game.h>
#include <ssod/game_player.h>
#include <ssod/game_enums.h>
#include <ssod/database.h>
#include <ssod/paragraph.h>

void game_nav(const dpp::button_click_t& event) {
	if (!player_is_live(event)) {
		return;
	}
	player p = get_live_player(event);
	if (event.custom_id.substr(0, 11) == "follow_nav;" && p.state == state_play) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.custom_id, ";");
		if (parts.size() < 3) {
			return;
		}
		p.paragraph = atol(parts[1].c_str());
		p.event.delete_original_response();
		p.event = event;
		update_live_player(event, p);
		p.save(event.command.usr.id);
		continue_game(event, p);
	}
};

void continue_game(const dpp::interaction_create_t& event, player p) {
	paragraph location(p.paragraph, p, event.command.usr.id);
	dpp::cluster& bot = *(event.from->creator);
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = "Location " + std::to_string(p.paragraph),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_description(location.text);

	dpp::message m;
	if (location.navigation_links.size()) {
		m.add_embed(embed).add_component(dpp::component());
		size_t index = 0;
		size_t component_parent = 0;
		for (const auto & n : location.navigation_links) {
			m.components[component_parent].add_component(
				dpp::component()
					.set_type(dpp::cot_button)
					.set_id("follow_nav;" + std::to_string(n) + ";" + std::to_string(p.paragraph))
					.set_label("Travel")
					.set_style(dpp::cos_primary)
					.set_emoji(directions[++index], 0, false)
			);
			if (index && (index % 5 == 0)) {
				m.add_component(dpp::component());
				component_parent++;
			}
		}
		if (m.components[component_parent].components.empty()) {
			m.components.erase(m.components.end() - 1);
		}
	}
	event.reply(m.set_flags(dpp::m_ephemeral), [event, &bot, location](const auto& cc) {
		if (cc.is_error()) {
			event.reply("Internal error displaying location " + std::to_string(location.id) + ":\n```json\n" + cc.http_info.body + "\n```");
		}
	});
}
