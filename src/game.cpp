#include <dpp/dpp.h>
#include <string>
#include <ssod/game.h>
#include <ssod/game_player.h>
#include <ssod/game_enums.h>
#include <ssod/database.h>
#include <ssod/paragraph.h>
#include <ssod/game_util.h>

void game_nav(const dpp::button_click_t& event) {
	if (!player_is_live(event)) {
		return;
	}
	player p = get_live_player(event);
	if (p.state != state_play || event.custom_id.empty()) {
		return;
	}
	std::vector<std::string> parts = dpp::utility::tokenize(event.custom_id, ";");
	if ((parts[0] == "follow_nav" || parts[0] == "follow_nav_pay") && parts.size() >= 3) {
		if (parts[0] == "follow_nav_pay" && parts.size() >= 4) {
			long link_cost = atol(parts[3].c_str());
			if (p.gold < link_cost) {
				return;
			}
			p.gold -= link_cost;
		} 
		p.paragraph = atol(parts[1].c_str());
	} else if (parts[0] == "shop" && parts.size() >= 6) {
		std::string flags = parts[3];
		long cost = atol(parts[4].c_str());
		std::string name = parts[5];
		p.paragraph = atol(parts[1].c_str());
		if (p.gold >= cost) {
			p.gold -= cost;
			if (flags == "SPELL") {
				p.spells.push_back(item{ .name = name, .flags = flags });
			} else if (flags == "HERB") {
				p.herbs.push_back(item{ .name = name, .flags = flags });
			} else {
				p.possessions.push_back(item{ .name = name, .flags = flags });
			}
		}
	}
	p.event.delete_original_response();
	p.event = event;
	update_live_player(event, p);
	p.save(event.command.usr.id);
	continue_game(event, p);
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
	p.save(event.command.usr.id);
	update_live_player(event, p);
	dpp::message m;
	m.add_embed(embed).add_component(dpp::component());
	size_t index = 0;
	size_t component_parent = 0;
	if (location.navigation_links.size()) {
		for (const auto & n : location.navigation_links) {
			std::string label{"Travel"}, id;
			dpp::component comp;
			if (n.type == nav_type_disabled_link || (p.gold < n.cost && n.type == nav_type_paylink) || (p.gold < n.cost && n.type == nav_type_shop)) {
				comp.set_disabled(true);
			}
			switch (n.type) {
				case nav_type_paylink:
					label = "Pay " + std::to_string(n.cost) + " Gold";
					id = "follow_nav_pay;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::to_string(n.cost);
					break;
				case nav_type_shop:
					label = "Buy " + n.buyable.name + " (" + std::to_string(n.cost) + " Gold)";
					id = "shop;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::string(n.buyable.flags) + ";" + std::to_string(n.cost) + ";" + n.buyable.name;
					if (p.has_herb(n.buyable.name) || p.has_spell(n.buyable.name) || p.gold < n.cost) {
						comp.set_disabled(true);
					}
					break;
				case nav_type_bank:
					label = "Use Bank";
					id = "bank;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph);
					break;
				case nav_type_combat:
					label = "Fight " + n.monster.name;
					id = "combat;" + std::to_string(n.paragraph) + ";" + n.monster.name + ";" + std::to_string(n.monster.stamina) + ";" + std::to_string(n.monster.skill) + ";" + std::to_string(n.monster.armour) + ";" + std::to_string(n.monster.weapon);
					break;
				default:
					id = "follow_nav;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph);
					break;
			}
			comp.set_type(dpp::cot_button)
				.set_id(id)
				.set_label(label)
				.set_style(n.type == nav_type_combat ? dpp::cos_danger : dpp::cos_primary)
				.set_emoji(directions[++index], 0, false);

			m.components[component_parent].add_component(comp);
			if (index && (index % 5 == 0)) {
				m.add_component(dpp::component());
				component_parent++;
			}
		}
		m.components[component_parent].add_component(help_button());
		index++;
		if (index && (index % 5 == 0)) {
			m.add_component(dpp::component());
			component_parent++;
		}
		if (m.components[component_parent].components.empty()) {
			m.components.erase(m.components.end() - 1);
		}
	}
	event.reply(m.set_flags(dpp::m_ephemeral), [m, event, &bot, location](const auto& cc) {
		if (cc.is_error()) {
			event.reply("Internal error displaying location " + std::to_string(location.id) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
		}
	});
}
