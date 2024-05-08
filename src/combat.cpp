/************************************************************************************
 * 
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023 Craig Edwards <brain@ssod.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#include <dpp/dpp.h>
#include <ssod/combat.h>
#include <ssod/component_builder.h>
#include <ssod/game_util.h>
#include <ssod/game_dice.h>
#include <ssod/ssod.h>
#include <ssod/emojis.h>
#include <ssod/aes.h>
#include <fmt/format.h>
#include <ssod/game.h>

const time_t combat_timeout = 60 * 5;

std::map<dpp::snowflake, combat_state> pvp_list;
std::mutex pvp_list_lock;

void remove_pvp(const dpp::snowflake id) {
	std::lock_guard<std::mutex> l(pvp_list_lock);
	auto p1 = pvp_list.find(id);
	if (p1 != pvp_list.end()) {
		dpp::snowflake o = p1->second.opponent;
		pvp_list.erase(p1);
		auto p2 = pvp_list.find(o);
		if (p2 != pvp_list.end()) {
			pvp_list.erase(p2);
		}
	}
}

player get_pvp_opponent(const dpp::snowflake id, dpp::discord_client* shard) {
	std::lock_guard<std::mutex> l(pvp_list_lock);
	auto p1 = pvp_list.find(id);
	if (p1 != pvp_list.end()) {
		dpp::interaction_create_t tmp(shard, "");
		tmp.command.usr.id = p1->second.opponent;
		return get_live_player(tmp, false);
	}
	return player();
}

void challenge_pvp(const dpp::interaction_create_t& event, const dpp::snowflake opponent) {
	player p = get_live_player(event, false);
	{
		std::lock_guard<std::mutex> l(pvp_list_lock);
		pvp_list[event.command.usr.id] = {
			.opponent = opponent,
			.accepted = false,
			.my_turn = false,
			.last_updated = time(nullptr),
		};
		pvp_list[opponent] = {
			.opponent = event.command.usr.id,
			.accepted = false,
			.my_turn = false,
			.last_updated = time(nullptr),
		};
	}
	player p2 = get_pvp_opponent(event.command.usr.id, event.from);
	send_chat(event.command.usr.id, p.paragraph, p2.name, "combat");
	dpp::message m = dpp::message(_("CHALLENGE_PVP", event, opponent.str(), p.name)).set_allowed_mentions(true, false, false, false, {}, {});
	m.channel_id = p2.event.command.channel_id;
	m.guild_id = p2.event.command.guild_id;
	m.add_component(
		dpp::component()
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("pvp_accept;" + event.command.usr.id.str() + ";" + opponent.str()))
			.set_label(_("ACCEPT", event))
			.set_style(dpp::cos_success)
			.set_emoji(sprite::sword008.name, sprite::sword008.id)
		)
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("pvp_reject;" + event.command.usr.id.str() + ";" + opponent.str()))
			.set_label(_("REJECT", event))
			.set_style(dpp::cos_danger)
			.set_emoji(sprite::magic05.name, sprite::magic05.id)
		)
	);
	
	//event.from->creator->message_create(m);
	if (p2.event.from) {
		p2.event.edit_original_response(m);
	}
}

dpp::snowflake get_pvp_opponent_id(const dpp::snowflake id) {
	std::lock_guard<std::mutex> l(pvp_list_lock);
	auto p1 = pvp_list.find(id);
	if (p1 != pvp_list.end()) {
		return p1->second.opponent;
	}
	return 0ull;
}

void update_save_opponent(const dpp::interaction_create_t& event, player p) {
	dpp::snowflake o = get_pvp_opponent_id(event.command.usr.id);
	dpp::interaction_create_t tmp(event.from, "");
	tmp.command.usr.id = o;
	update_live_player(tmp, p);
	p.save(o);
}

player set_in_pvp_combat(const dpp::interaction_create_t& event) {
	player p1 = get_live_player(event, false);
	p1.in_combat = true;
	player p2 = get_pvp_opponent(event.command.usr.id, event.from);
	dpp::snowflake oid = get_pvp_opponent_id(event.command.usr.id);
	dpp::interaction_create_t tmp(event.from, "");
	tmp.command.usr.id = oid;
	p2.in_combat = true;
	p1.challenged_by = oid;
	p2.challenged_by = event.command.usr.id;
	update_live_player(event, p1);
	update_live_player(tmp, p2);
	return p1;
}

void update_opponent_message(const dpp::interaction_create_t& event, dpp::message m, const std::stringstream& output) {
	m.embeds[0].description += output.str();
	if (has_active_pvp(event.command.usr.id)) {
		player opponent = get_pvp_opponent(event.command.usr.id, event.from);
		if (opponent.event.from) {
			opponent.event.edit_original_response(m, [event, o = opponent](const auto& cc) {
				if (cc.is_error()) {
					player opponent = o;
					player p = get_live_player(event, false);
					if (opponent.stamina > 0 && p.stamina > 0) {
						opponent.stamina = 0;
						opponent.in_pvp_picker = false;
						p.in_pvp_picker = false;
						opponent.save(event.command.usr.id);
						update_live_player(event, p);
						update_save_opponent(event, opponent);
						update_opponent_message(event, get_pvp_round(opponent.event), std::stringstream(_("TIMEOUT", event, opponent.name)));
						update_opponent_message(opponent.event, get_pvp_round(event), std::stringstream(_("TIMEOUT", event, opponent.name)));
						p = end_pvp_combat(event);
						/* To the victor go the spoils */
						p.add_experience(opponent.xp_worth());
						send_chat(opponent.event.command.usr.id, p.paragraph, _("WHIMS", event), "death");
						update_save_opponent(event, opponent);
					}
				}
			});	
		}
	}	
}

void accept_pvp(const dpp::snowflake id1, const dpp::snowflake id2) {
	std::lock_guard<std::mutex> l(pvp_list_lock);
	bool turn = random(0, 1);
	if (id1 != 0 && id2 != 0) {
		pvp_list[id2] = {
			.opponent = id1,
			.accepted = true,
			.my_turn = turn,
			.last_updated = time(nullptr),
		};
		pvp_list[id1] = {
			.opponent = id2,
			.accepted = true,
			.my_turn = !turn,
			.last_updated = time(nullptr),
		};
	}
}

player end_pvp_combat(const dpp::interaction_create_t& event) {
	player p1 = get_live_player(event, false);
	p1.in_combat = false;
	p1.challenged_by = 0;
	player p2 = get_pvp_opponent(event.command.usr.id, event.from);
	dpp::snowflake oid = get_pvp_opponent_id(event.command.usr.id);
	dpp::interaction_create_t tmp(event.from, "");
	tmp.command.usr.id = oid;
	p2.in_combat = false;
	p2.challenged_by = 0;
	update_live_player(event, p1);
	update_live_player(tmp, p2);
	remove_pvp(event.command.usr.id);
	return p1;
}

bool has_active_pvp(const dpp::snowflake id) {
	std::lock_guard<std::mutex> l(pvp_list_lock);
	auto p = pvp_list.find(id);
	return (p != pvp_list.end() && p->second.accepted);
}

bool is_my_pvp_turn(const dpp::snowflake id) {
	std::lock_guard<std::mutex> l(pvp_list_lock);
	auto p = pvp_list.find(id);
	return (p != pvp_list.end() && p->second.accepted && p->second.my_turn);
}

void swap_pvp_turn(const dpp::snowflake id) {
	std::lock_guard<std::mutex> l(pvp_list_lock);
	auto p = pvp_list.find(id);
	if (p != pvp_list.end() && p->second.accepted) {
		p->second.my_turn = !p->second.my_turn;
		p->second.last_updated = time(nullptr);
		auto p2 = pvp_list.find(p->second.opponent);
		if (p2 != pvp_list.end()) {
			p2->second.my_turn = !p2->second.my_turn;
			p2->second.last_updated = time(nullptr);
		}
	}
}

void end_abandoned_pvp() {
	time_t now = time(nullptr);
	std::map<dpp::snowflake, combat_state> pvp_list_copy;
	{
		std::lock_guard<std::mutex> l(pvp_list_lock);
		pvp_list_copy = pvp_list;
	}
	for (const auto& [id, pvp] : pvp_list_copy) {
		if (now - pvp.last_updated > combat_timeout && pvp.my_turn && pvp.accepted) {
			/* Five minutes without action, PvP is forfeit to the other player */
			dpp::interaction_create_t event(nullptr, "");
			event.command.usr.id = id;
			player p = get_live_player(event, false);
			event.from = p.event.from;
			player opponent = get_pvp_opponent(event.command.usr.id, event.from);
			if (opponent.stamina > 0 && p.stamina > 0) {
				p.stamina = 0;
				p.in_pvp_picker = false;
				opponent.in_pvp_picker = false;
				p.save(event.command.usr.id);
				update_live_player(event, p);
				update_save_opponent(event, opponent);
				update_opponent_message(event, get_pvp_round(opponent.event), std::stringstream(_("TIMEOUT5", event, p.name)));
				update_opponent_message(opponent.event, get_pvp_round(event), std::stringstream(_("TIMEOUT5", event, p.name)));
				p = end_pvp_combat(event);
				/* To the victor go the spoils */
				opponent.add_experience(p.xp_worth());
				send_chat(event.command.usr.id, p.paragraph, _("RAVAGES", event), "death");
				update_save_opponent(event, opponent);
			}
		}
	}
}

long get_spell_rating(const std::string& name)
{
	static const std::map<std::string, long> ratings = {
		{"fire", 2},
		{"water", 4},
		{"strength", 2},
		{"bolt", 4},
		{"fasthands", 2},
		{"thunderbolt", 6},
		{"heateyes", 4},
		{"espsurge", 5},
		{"afterimage", 4},
		{"growweapon", 4},
		{"vortex", 10},
	};
	auto r = ratings.find(dpp::lowercase(name));
	if (r != ratings.end()) {
		return r->second;
	}
	return 0;
}


dpp::message get_pvp_round(const dpp::interaction_create_t& event) {
	dpp::message m;
	component_builder cb(m);
	std::stringstream output;
	player opponent = get_pvp_opponent(event.command.usr.id, event.from);
	player p = get_live_player(event, false);
	bool turn = is_my_pvp_turn(event.command.usr.id);

	output << "### " << p.name << " vs " << opponent.name << "\n";

	if (turn) {
		if (p.stamina > 0) {
			output << _("YOUR_TURN", event) << " " << dpp::utility::timestamp(time(nullptr) + combat_timeout, dpp::utility::tf_relative_time) << "!";
			size_t index = 0;
			for (const auto & inv :  p.possessions) {
				if (inv.flags.length() >= 2 && inv.flags[0] == 'W' && isdigit(inv.flags[1])) {
					dpp::emoji e = get_emoji(inv.name, inv.flags);
					cb.add_component(dpp::component()
						.set_type(dpp::cot_button)
						.set_id(security::encrypt("pvp_attack;" + inv.name + ";" + inv.flags.substr(1, inv.flags.length() - 1) + ";" + std::to_string(++index)))
						.set_label(_("ATTACK_USING", event, inv.name))
						.set_style(dpp::cos_secondary)
						.set_emoji(e.name, e.id)
					);
				}
			}
			for (const auto & spell :  p.spells) {
				long rating = get_spell_rating(spell.name);
				if (rating) {
					dpp::emoji e = sprite::hat02;
					cb.add_component(dpp::component()
						.set_type(dpp::cot_button)
						.set_id(security::encrypt("pvp_attack;" + spell.name + ";" + std::to_string(rating) + ";" + std::to_string(++index)))
						.set_label(_("CAST", event, spell.name))
						.set_style(dpp::cos_secondary)
						.set_emoji(e.name, e.id)
						.set_disabled(p.mana < rating)
					);
				}
			}
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("pvp_change_stance;" + std::string(p.stance == DEFENSIVE ? "o" : "d")))
				.set_label(_("STANCE", event) + ": " + std::string(_(p.stance == DEFENSIVE ? "DEFENSIVE" : "OFFENSIVE", event)) + " " + _("CLICK_TO_CHANGE", event))
				.set_style(dpp::cos_secondary)
				.set_emoji(sprite::wood03.name, sprite::wood03.id)
			);
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("pvp_change_strike;" + std::string(p.attack == CUTTING ? "p" : "c")))
				.set_label(_("ATTACK_TYPE", event) + ": " + std::string(_(p.attack == CUTTING ? "CUTTING" : "PIERCING", event)) + " " + _("CLICK_TO_CHANGE", event))
				.set_style(dpp::cos_secondary)
				.set_emoji(sprite::shoes04.name, sprite::shoes04.id)
			);
		}
	} else {
		if (p.stamina > 0) { 
			output << _("OTHER_TURN", event, opponent.name, dpp::utility::timestamp(time(nullptr) + combat_timeout, dpp::utility::tf_relative_time));
		}
	}
	if (p.stamina < 1) {
		death(p, cb);
		p.save(event.command.usr.id);
		update_live_player(event, p);
	}
	else if (opponent.stamina < 1) {
		p.in_combat = p.in_pvp_picker = false;
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("follow_nav;" + std::to_string(p.paragraph) + ";" + std::to_string(p.paragraph)))
			.set_label(_("VICTORY", event))
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::sword_box_green.name, sprite::sword_box_green.id)
		);
	}

	output << "\nYour Stance: **" << (p.stance == DEFENSIVE ? "defensive " + sprite::wood03.get_mention() : "offensive " + sprite::sword008.get_mention()) << "**";
	std::stringstream output1, output2;
	output1 << "\n\n```ansi\n";
	output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("SKILL", event), p.skill) << "\n";
	output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("STAMINA", event), p.stamina) << "\n";
	output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("ARMOUR", event), p.armour.rating) << "\n";
	output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("WEAPON", event), p.weapon.rating) << "\n";
	output1 << "```\n\n";
	output2 << "\n\n```ansi\n";
	output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("SKILL", event), opponent.skill) << "\n";
	output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("STAMINA", event), opponent.stamina) << "\n";
	output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("ARMOUR", event), opponent.armour.rating) << "\n";
	output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("WEAPON", event), opponent.weapon.rating) << "\n";
	output2 << "```\n\n";

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = _("INPVP", event, opponent.name, opponent.paragraph),
			.icon_url = "", 
			.proxy_url = "",
		})
		.add_field(p.name.substr(0, 80),output1.str(), true)
		.add_field(opponent.name.substr(0, 80),output2.str(), true)
		.set_colour(EMBED_COLOUR)
		.set_description(output.str());
	
	m = cb.get_message();
	m.add_embed(embed);

	do_toasts(p, cb);

	return m;
}

void continue_pvp_combat(const dpp::interaction_create_t& event, player p, const std::stringstream& output) {

	dpp::message m(get_pvp_round(event));
	m.embeds[0].description += output.str();

	event.reply(event.command.type == dpp::it_component_button ? dpp::ir_update_message : dpp::ir_channel_message_with_source, m.set_flags(dpp::m_ephemeral), [event, m, p](const auto& cc) {
		if (cc.is_error()) {
			//bot.log(dpp::ll_error, "Internal error displaying PvP combat location " + std::to_string(p.paragraph) + ": " + cc.http_info.body);
			event.reply(dpp::message("Internal error displaying Pvp combat location " + std::to_string(p.paragraph) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```").set_flags(dpp::m_ephemeral));
		}
	});
}

bool pvp_combat_nav(const dpp::button_click_t& event, player p, const std::vector<std::string>& parts) {
	if (!p.in_combat && !has_active_pvp(event.command.usr.id)) {
		return false;
	}
	bool claimed{false};
	player opponent = get_pvp_opponent(event.command.usr.id, event.from);
	dpp::snowflake oid = get_pvp_opponent_id(event.command.usr.id);
	std::stringstream output1, output2;

	if (parts[0] == "pvp_accept") {
		/* Fall-through to prevent going to PvE combat code after accept */
		claimed = true;
	} else if (parts[0] == "pvp_attack" && is_my_pvp_turn(event.command.usr.id) && parts.size() >= 3) {
		p.weapon.rating = atol(parts[2]);
		p.weapon.name = parts[1];
		if (p.has_spell(p.weapon.name)) {
			if (p.mana < p.weapon.rating) {
				p.weapon.rating = 0;
				p.weapon.name = _("NOMANA", event);
			} else {
				p.mana -= p.weapon.rating;
			}
		}
		/* Deal damage + saving throws */
		long PAttack = dice() + dice() + p.skill + p.weapon.rating;
		if ((p.stance == OFFENSIVE) && (opponent.stance == DEFENSIVE)) {
			int Bonus = dice();
			PAttack += Bonus;
			output1 << _("PVPOFF", event, opponent.name, Bonus);
			output2 << _("PFPDEF", event, p.name, Bonus);
		}
		output1 << _("PVPATKME", event, PAttack, p.weapon.name) << "\n\n";
		output2 << _("PVPATKOP", event, p.name, PAttack, p.weapon.name) << "\n\n";
		long SaveRoll = dice() + dice();
		bool Saved = false;		
		if (opponent.stance == DEFENSIVE) {
			output1 << " " << _("PVPOPDEF", event, opponent.name);
			output2 << " " << _("PVPMEDEF", event);
			SaveRoll -= dice();
		}
		if (SaveRoll <= opponent.armour.rating) {
			Saved = true;
		}
		long D6 = dice();
		long SDamage{}, KDamage{};
		combat_strike KAttackType = p.attack;
		if (Saved) {
			output1 << _("PVPOPARMOUR", event, opponent.armour.name);
			output2 << _("PVPMEARMOUR", event, opponent.armour.name);
		} else {
			output1 << _("PVPBREAKOP", event, opponent.armour.name);
			output2 << _("PVPBREAKME", event, opponent.armour.name);
			switch (D6) {
				case 1:
					output1 << _("HEAD", event);
					output2 << _("HEAD", event);
					SDamage = dice();
					KDamage = 1;
					break;
				case 2:
					output1 << _("LEGS", event);
					output2 << _("LEGS", event);
					SDamage = 3;
					KDamage = 1;
					break;
				case 3:
					output1 << _("TORSO", event);
					output2 << _("TORSO", event);
					SDamage = dice();
					KDamage = 0;
					break;
				case 4:
					output1 << _("ARMS", event);
					output2 << _("ARMS", event);
					SDamage = 2;
					KDamage = 2;
					break;
				case 5:
					output1 << _("HANDS", event);
					output2 << _("HANDS", event);
					SDamage = 2;
					KDamage = 1;
					break;
				case 6:
					output1 << _("BWEAPON", event);
					output2 << _("BWEAPON", event);
					SDamage = 0;
					KDamage = 1;
					break;
			}
			output1 << "** " << _("AREA", event) << ", ";
			output2 << "** " << _("AREA", event) << ", ";
			switch (D6) {
				case 1:
					if (KAttackType == CUTTING) {
						output1 << _("CUTTINGOUTCOME", event);
						output2 << _("CUTTINGOUTCOME", event);
						SDamage += dice();
					}
					break;
				case 2:
					if (KAttackType == PIERCING) {
						output1 << _("PIERCINGOUTCOME", event);
						output2 << _("PIERCINGOUTCOME", event);
						SDamage += dice();
					}
					break;
			}
			if (SDamage == 0) {
				output1 << _("NOSTAMINALOSS", event) << " ";
				output2 << _("NOSTAMINALOSS", event) << " ";
			} else {
				output1 << _("STAMINALOSS", event, SDamage) << " ";
				output2 << _("STAMINALOSS", event, SDamage) << " ";
			}
			if (KDamage == 0) {
				output1 << _("NOSKILLLOSS", event) << " ";
				output2 << _("NOSKILLLOSS", event) << " ";
			} else {
				output1 << _("SKILLLOSS", event, KDamage) << " ";;
				output2 << _("SKILLLOSS", event, KDamage) << " ";;
			}
			opponent.stamina -= SDamage;
			opponent.skill -= KDamage;
			p.strike();			
			if (p.stamina < 4) {
				output1 << _("YOU_DYING", event) << " ";
				output2 << _("ENEMY_FOCUS", event) << " ";
			} else if (opponent.stamina < 4) {
				output1 << _("ENEMY_DYING", event) << " ";
				output2 << _("YOU_DYING", event) << " ";
			}
			if (p.skill < 5) {
				output1 << _("YOU_FOCUS", event) << " ";
			} else if (opponent.skill < 5) {
				output2 << _("ENEMY_FOCUS", event) << " ";
			}
			if (opponent.stamina < 1 || p.stamina < 1) {

				const size_t max_death_messages = 44;
				std::string death_message = _(fmt::format("DEATH_MSG_{}", random(0, max_death_messages)), event);
				if (p.stamina < 1) {
					output1 << fmt::format(fmt::runtime(death_message), p.name, opponent.name);
					output2 << fmt::format(fmt::runtime(death_message), p.name, opponent.name);
				} else {
					output1 << fmt::format(fmt::runtime(death_message), p.name, opponent.name);
					output2 << fmt::format(fmt::runtime(death_message), p.name, opponent.name);
				}
			}
		}			
		swap_pvp_turn(event.command.usr.id);
		claimed = true;
	} else if (parts[0] == "pvp_change_stance" && parts.size() >= 2) {
		p.stance = (parts[1] == "o" ? OFFENSIVE : DEFENSIVE);
		claimed = true;
	} else if (parts[0] == "pvp_change_strike" && parts.size() >= 2) {
		p.attack = (parts[1] == "p" ? PIERCING : CUTTING);
		claimed = true;
	}

	if (claimed) {
		p.save(event.command.usr.id);
		update_live_player(event, p);
		update_save_opponent(event, opponent);
		update_opponent_message(event, get_pvp_round(opponent.event), output2);
		continue_pvp_combat(event, p, output1);
		if (p.stamina < 1 || opponent.stamina < 1) {
			p = end_pvp_combat(event);
			/* To the victor go the spoils */
			if (opponent.stamina < 1) {
				p.add_experience(opponent.xp_worth());
				send_chat(oid, opponent.paragraph, p.name, "death");
			} else {
				opponent.add_experience(p.xp_worth());
				send_chat(event.command.usr.id, p.paragraph, opponent.name, "death");
			}
			p.save(event.command.usr.id);
			update_live_player(event, p);
			update_save_opponent(event, opponent);
		}
		return true;
	}
	return false;

}

bool combat_nav(const dpp::button_click_t& event, player p, const std::vector<std::string>& parts) {
	if (!p.in_combat) {
		return false;
	}
	if (pvp_combat_nav(event, p, parts)) {
		return true;
	}
	bool claimed{false};

	if (parts[0] == "attack" && parts.size() >= 3) {
		p.weapon.rating = atol(parts[2]);
		p.weapon.name = parts[1];
		claimed = true;
	} else if (parts[0] == "change_stance" && parts.size() >= 2) {
		p.stance = (parts[1] == "o" ? OFFENSIVE : DEFENSIVE);
		claimed = true;
	} else if (parts[0] == "change_strike" && parts.size() >= 2) {
		p.attack = (parts[1] == "p" ? PIERCING : CUTTING);
		claimed = true;
	}
	if (p.has_spell(p.weapon.name)) {
		if (p.mana < p.weapon.rating) {
			p.weapon.rating = 0;
			p.weapon.name = _("NOMANA", event);
		} else {
			p.mana -= p.weapon.rating;
		}
	}

	if (claimed) {
		continue_combat(event, p);
		return true;
	}
	return false;
}


void continue_combat(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	dpp::message m;
	component_builder cb(m);
	std::stringstream output1, output2;

	long PArmour = p.armour.rating, PWeapon = p.weapon.rating;
	long& ESkill = p.combatant.skill;
	long& EStamina = p.combatant.stamina;
	long& EArmour = p.combatant.armour;
	long& EWeapon = p.combatant.weapon;
	std::stringstream output;

	if ((int)p.attack == 0 || (int)p.stance == 0 || (int)p.stance > 2 || (int)p.attack > 2) {
		p.attack = CUTTING;
		p.stance = OFFENSIVE;
	}
	combat_strike EAttackType = PIERCING;
	combat_stance EStance = DEFENSIVE;
	std::string WeaponFlags;

	if (dice() >= 3) {
		EAttackType = CUTTING;
	}
	if (dice() >= 3) {
		EStance = OFFENSIVE;
	}


	output << "__" << _("COMBAT", event) << "__: **" << p.name << "** vs. **" << p.combatant.name << "**\n\n";

	if (EStamina <= 0) {
		output << _("HES_DEAD_JIM", event) << "\n\n";
		p.after_fragment++;
		p.combatant = {};
		p.in_combat = false;
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("follow_nav;" + std::to_string(p.paragraph) + ";" + std::to_string(p.paragraph)))
			.set_label(_("VICTORY", event))
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::sword_box_green.name, sprite::sword_box_green.id)
		);
		output1 << "\n\n```ansi\n";
		output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("SKILL", event), p.skill) << "\n";
		output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("STAMINA", event), p.stamina) << "\n";
		output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("ARMOUR", event), p.armour.rating) << "\n";
		output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("WEAPON", event), p.weapon.rating) << "\n";
		output1 << "```\n\n";
		output2 << "\n\n```ansi\n";
		output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("SKILL", event), p.combatant.skill) << "\n";
		output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("STAMINA", event), p.combatant.stamina) << "\n";
		output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("ARMOUR", event), p.combatant.armour) << "\n";
		output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("WEAPON", event), p.combatant.weapon) << "\n";
		output2 << "```\n\n";
	} else {

		long EAttack = dice() + dice() + ESkill + EWeapon;
		long PAttack = dice() + dice() + p.skill + PWeapon;

		output << _("ENEMYSTANCE", event) << " **" << (EStance == DEFENSIVE ? _("DEFENSIVE", event) + " " + sprite::wood03.get_mention() : _("OFFENSIVE", event) + " " + sprite::sword008.get_mention()) << "**\n";
		output << _("YOURSTANCE", event) << " **" << (p.stance == DEFENSIVE ? _("DEFENSIVE", event) + " " + sprite::wood03.get_mention() : _("OFFENSIVE", event) + " " + sprite::sword008.get_mention()) << "**\n";

		if ((EStance == OFFENSIVE) && (p.stance == DEFENSIVE)) {
			int Bonus = dice();
			EAttack += Bonus;
			output << _("COWERING", event, Bonus);
		} else  if ((p.stance == OFFENSIVE) && (EStance == DEFENSIVE)) {
			int Bonus = dice();
			PAttack += Bonus;
			output << _("LOOMING", event, Bonus);
		}
	
		output << "\n\n" << _("PVETOTALS", event, PAttack, EAttack) << "\n\n";

		long SaveRoll = dice() + dice();
		bool Saved = false;

		if (EAttack > PAttack) {
			output << "__**" << _("ENEMYHITSYOU", event) << "**__. ";
			
			if (p.stance == DEFENSIVE) {
				output << _("SAVEBONUS", event);
				SaveRoll -= dice();
			}
			if (SaveRoll <= PArmour) {
				Saved = true;
			}
		}
		else
		{
			output << "__**" << _("YOUHITENEMY", event) << "**__.";
			
			if (EStance == DEFENSIVE) {
				output << _("ATKBONUS", event);
				SaveRoll -= dice();
			}
			if (SaveRoll <= EArmour) {
				Saved = true;
			}
		}
		long D6 = dice();
		long SDamage{}, KDamage{};
		combat_strike KAttackType = (EAttack > PAttack ? EAttackType : p.attack);

		if (Saved) {
			output << " " << _("PVESAVED", event);
		} else {

			output << _("PVENOSAVE", event) << " **";
			switch (D6) {
				case 1:
					output << _("HEAD", event);
					SDamage = dice();
					KDamage = 1;
					break;
				case 2:
					output << _("LEGS", event);
					SDamage = 3;
					KDamage = 1;
					break;
				case 3:
					output << _("TORSO", event);
					SDamage = dice();
					KDamage = 0;
					break;
				case 4:
					output << _("ARMS", event);
					SDamage = 2;
					KDamage = 2;
					break;
				case 5:
					output << _("HANDS", event);
					SDamage = 2;
					KDamage = 1;
					break;
				case 6:
					output << _("BWEAPON", event);
					SDamage = 0;
					KDamage = 1;
					break;
			}

			output << "** " << _("AREA", event) << ".";;


			switch (D6) {
				case 1:
					if (KAttackType == CUTTING) {
						output << _("CUTTING_CONSEQUENCE", event);
						SDamage += dice();
					}
					break;
				case 2:
					if (KAttackType == PIERCING) {
						output << _("PIERCING_CONSEQUENCE", event);
						SDamage += dice();
					}
					break;
			}

			if (SDamage == 0) {
				output << _("NOSTAMINALOSS", event) << " ";
			} else {
				output << _("STAMINALOSS", event, SDamage) << " ";
			}
			if (KDamage == 0) {
				output << _("NOSKILLLOSS", event) << " ";
			} else {
				output << _("SKILLLOSS", event, KDamage) << " ";;
			}

			if (EAttack > PAttack) {
				p.stamina -= SDamage;
				p.skill -= KDamage;
			} else {
				EStamina -= SDamage;
				ESkill -= KDamage;
				p.strike();
			}

			if (p.stamina < 4) {
				output << _("YOU_DYING", event) << " ";
			} else if (EStamina < 4) {
				output << _("ENEMY_DYING", event) << " ";
			}

			if (p.skill < 5) {
				output << _("YOU_FOCUS", event) << " ";
			} else if (ESkill < 5) {
				output << _("ENEMY_FOCUS", event) << " ";
			}


			if (EStamina < 1 || p.stamina < 1) {
				const size_t max_death_messages = 44;
				std::string death_message = _(fmt::format("DEATH_MSG_{}", random(0, max_death_messages)), event);
				if (p.stamina < 1) {
					output << fmt::format(fmt::runtime(death_message), p.name, p.combatant.name);
				} else {
					/* Add experience on victory equal to remaining skill of enemy (indicates difficulty of the fight) */
					long xp = abs(std::max(p.combatant.skill, 0l) * 0.15f) + 1;
					output << "\n\n***+" + std::to_string(xp) + " XP!***\n\n";
					p.add_experience(xp);
					output << "**" << fmt::format(fmt::runtime(death_message), p.combatant.name, p.name) << "**";
				}
			}
		}

		output1 << "\n\n```ansi\n";
		output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("SKILL", event), p.skill) << "\n";
		output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("STAMINA", event), p.stamina) << "\n";
		output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("ARMOUR", event), p.armour.rating) << "\n";
		output1 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("WEAPON", event), p.weapon.rating) << "\n";
		output1 << "```\n\n";
		output2 << "\n\n```ansi\n";
		output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("SKILL", event), p.combatant.skill) << "\n";
		output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("STAMINA", event), p.combatant.stamina) << "\n";
		output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("ARMOUR", event), p.combatant.armour) << "\n";
		output2 << fmt::format(fmt::runtime("\033[2;31m{0}\033[0m: \033[2;33m{1:2d}\033[0m"), _("WEAPON", event), p.combatant.weapon) << "\n";
		output2 << "```\n\n";

		bool CombatEnded = false;
		if (p.stamina < 1) {
			death(p, cb);
			p.save(event.command.usr.id);
			update_live_player(event, p);
			CombatEnded = true;
		} else if (EStamina < 1) {
			p.after_fragment++;
			p.combatant = {};
			p.in_combat = false;
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("follow_nav;" + std::to_string(p.paragraph) + ";" + std::to_string(p.paragraph)))
				.set_label(_("VICTORY", event))
				.set_style(dpp::cos_primary)
				.set_emoji(sprite::sword_box_green.name, sprite::sword_box_green.id)
			);
			CombatEnded = true;
		}

		if (!CombatEnded) {
			// todo: iterate inventory for anything with weapon score and present as option here,
			// also combat spells
			size_t index = 0;
			for (const auto & inv :  p.possessions) {
				if (inv.flags.length() >= 2 && inv.flags[0] == 'W' && isdigit(inv.flags[1])) {
					dpp::emoji e = get_emoji(inv.name, inv.flags);
					cb.add_component(dpp::component()
						.set_type(dpp::cot_button)
						.set_id(security::encrypt("attack;" + inv.name + ";" + inv.flags.substr(1, inv.flags.length() - 1) + ";" + std::to_string(++index)))
						.set_label(_("ATTACK_USING", event, inv.name))
						.set_style(dpp::cos_secondary)
						.set_emoji(e.name, e.id)
					);
				}
			}
			for (const auto & spell :  p.spells) {
				long rating = get_spell_rating(spell.name);
				if (rating) {
					dpp::emoji e = sprite::hat02;
					cb.add_component(dpp::component()
						.set_type(dpp::cot_button)
						.set_id(security::encrypt("attack;" + spell.name + ";" + std::to_string(rating) + ";" + std::to_string(++index)))
						.set_label(_("CAST", event, spell.name))
						.set_style(dpp::cos_secondary)
						.set_emoji(e.name, e.id)
						.set_disabled(p.mana < rating)
					);
				}
			}
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("change_stance;" + std::string(p.stance == DEFENSIVE ? "o" : "d")))
				.set_label(_("STANCE", event) + ": " + _(p.stance == DEFENSIVE ? "DEFENSIVE" : "OFFENSIVE", event) + " (click to change)")
				.set_style(dpp::cos_secondary)
				.set_emoji(sprite::wood03.name, sprite::wood03.id)
			);
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("change_strike;" + std::string(p.attack == CUTTING ? "p" : "c")))
				.set_label(_("ATTACK_TYPE", event) + ": " + _(p.attack == CUTTING ? "CUTTING" : "PIERCING", event) + " (click to change)")
				.set_style(dpp::cos_secondary)
				.set_emoji(sprite::shoes04.name, sprite::shoes04.id)
			);
		}
	}
	
	cb.add_component(help_button());

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = _("INCOMBAT", event, p.combatant.name, p.paragraph),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.add_field(p.name.substr(0, 80),output1.str(), true)
		.add_field(p.combatant.name.substr(0, 80),output2.str(), true)
		.set_description(output.str());

	std::array<std::string, 3> types{".png", ".jpg", ".webm"};
	for (const std::string& ext : types) {
		if (fs::exists("../resource/paragraph_pictures/" + std::to_string(p.paragraph) + ext)) {
			embed.set_image("https://images.ssod.org/resource/paragraph_pictures/" + std::to_string(p.paragraph) + ext);
			break;
		}
	}

	p.save(event.command.usr.id);
	update_live_player(event, p);
	cb.add_embed(embed);
	do_toasts(p, cb);
	m = cb.get_message();

	event.reply(event.command.type == dpp::it_component_button ? dpp::ir_update_message : dpp::ir_channel_message_with_source, m.set_flags(dpp::m_ephemeral), [event, &bot, m, p](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying PvE combat " + std::to_string(p.after_fragment) + " location " + std::to_string(p.paragraph) + ": " + cc.http_info.body + " -- " + m.build_json());
			event.reply(dpp::message("Internal error displaying PvE combat " + std::to_string(p.after_fragment) + " location " + std::to_string(p.paragraph) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```").set_flags(dpp::m_ephemeral));
		}
	});

}
