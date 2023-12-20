#include <dpp/dpp.h>
#include <string>
#include <ssod/ssod.h>
#include <ssod/game.h>
#include <ssod/game_player.h>
#include <ssod/game_enums.h>
#include <ssod/database.h>
#include <ssod/paragraph.h>
#include <ssod/game_util.h>
#include <ssod/component_builder.h>
#include <ssod/combat.h>

void game_nav(const dpp::button_click_t& event) {
	if (!player_is_live(event)) {
		return;
	}
	player p = get_live_player(event);
	bool claimed = false;
	if (p.state != state_play || event.custom_id.empty()) {
		return;
	}
	std::vector<std::string> parts = dpp::utility::tokenize(event.custom_id, ";");
	if (p.in_combat) {
		if (combat_nav(event, p, parts)) {
			return;
		}
	}
	if ((parts[0] == "follow_nav" || parts[0] == "follow_nav_pay") && parts.size() >= 3) {
		if (parts[0] == "follow_nav_pay" && parts.size() >= 4) {
			long link_cost = atol(parts[3]);
			if (p.gold < link_cost) {
				return;
			}
			p.gold -= link_cost;
		}
		p.after_fragment = 0; // Resets current combat index
		p.paragraph = atol(parts[1]);
		claimed = true;
	} else if (parts[0] == "shop" && parts.size() >= 6) {
		std::string flags = parts[3];
		long cost = atol(parts[4]);
		std::string name = parts[5];
		p.paragraph = atol(parts[1]);
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
		claimed = true;
	} else if (parts[0] == "combat" && parts.size() >= 7) {
		// paragraph name stamina skill armour weapon
		p.in_combat = true;
		p.combatant = enemy{
			.name = parts[2],
			.stamina = atol(parts[3]),
			.skill = atol(parts[4]),
			.armour = atol(parts[5]),
			.weapon = atol(parts[6]),
		};
		claimed = true;

	} else if (parts[0] == "respawn" && p.stamina < 1) {
		/* Load backup of player and save over the current */
		player new_p = player(event.command.usr.id, true);
		/* Keep experience points only (HARDCORE!!!) */
		new_p.experience = p.experience;
		new_p.in_combat = false;
		new_p.after_fragment = 0;
		new_p.combatant = {};
		update_live_player(event, new_p);
		new_p.save(event.command.usr.id);
		p = new_p;
		claimed = true;
	}
	if (claimed) {
		p.event = event;
		update_live_player(event, p);
		p.save(event.command.usr.id);
		continue_game(event, p);
	}
};

void continue_game(const dpp::interaction_create_t& event, player p) {
	if (p.in_combat) {
		continue_combat(event, p);
		return;
	}
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
	m.add_embed(embed);
	component_builder cb(m);
	size_t index{0}, enabled_links{0};
	bool respawn_button_shown{false};
	for (const auto & n : location.navigation_links) {
		std::string label{"Travel"}, id;
		dpp::component comp;
		if (respawn_button_shown && n.type == nav_type_respawn) {
			continue;
		}
		if (n.type == nav_type_disabled_link || (p.gold < n.cost && n.type == nav_type_paylink) || (p.gold < n.cost && n.type == nav_type_shop)) {
			comp.set_disabled(true);
		}
		switch (n.type) {
			case nav_type_paylink:
				label = "Pay " + std::to_string(n.cost) + " Gold";
				id = "follow_nav_pay;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::to_string(n.cost);
				enabled_links++;
				break;
			case nav_type_shop:
				label = "Buy " + n.buyable.name + " (" + std::to_string(n.cost) + " Gold)";
				id = "shop;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::string(n.buyable.flags) + ";" + std::to_string(n.cost) + ";" + n.buyable.name;
				if (p.has_herb(n.buyable.name) || p.has_spell(n.buyable.name) || p.gold < n.cost) {
					comp.set_disabled(true);
				} else {
					enabled_links++;
				}
				break;
			case nav_type_bank:
				label = "Use Bank";
				id = "bank;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph);
				enabled_links++;
				break;
			case nav_type_combat:
				label = "Fight " + n.monster.name;
				id = "combat;" + std::to_string(n.paragraph) + ";" + n.monster.name + ";" + std::to_string(n.monster.stamina) + ";" + std::to_string(n.monster.skill) + ";" + std::to_string(n.monster.armour) + ";" + std::to_string(n.monster.weapon);
				enabled_links++;
				break;
			case nav_type_respawn:
				label = "Respawn";
				id = "respawn";
				break;
			default:
				id = "follow_nav;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph);
				enabled_links++;
				break;
		}
		comp.set_type(dpp::cot_button)
			.set_id(id)
			.set_label(label)
			.set_style(n.type == nav_type_combat ? dpp::cos_danger : dpp::cos_primary)
			.set_emoji(directions[++index], 0, false);

		if (n.type == nav_type_respawn) {
			comp.set_emoji("💀", 0, false);
		}
		cb.add_component(comp);
	}
	if (enabled_links == 0 && !respawn_button_shown) {
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id("respawn")
			.set_label("Respawn")
			.set_style(dpp::cos_danger)
			.set_emoji("💀", 0, false)
		);
	}
	cb.add_component(help_button());
	m = cb.get_message();

	event.reply(event.command.type == dpp::it_component_button ? dpp::ir_update_message : dpp::ir_channel_message_with_source, m.set_flags(dpp::m_ephemeral), [event, &bot, location, m](const auto& cc) {
		if (cc.is_error()) {{
			event.reply("Internal error displaying location " + std::to_string(location.id) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
		}}
	});
}
