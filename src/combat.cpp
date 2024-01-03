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
#include <ssod/database.h>
#include <ssod/ssod.h>
#include <ssod/emojis.h>
#include <ssod/aes.h>
#include <fmt/format.h>
#include <ssod/game.h>

std::map<dpp::snowflake, combat_state> pvp_list;

const std::vector<std::string_view> death_messages{
	"{} departs the land of the living.",
	"{} travels to the next world courtesy of {}.",
	"{} lead a heroic, but brief life.",
	"The life of {} was cut short by {}.",
	"{} is now dancing with the reaper.",
	"{} wins a free trip across the styx from {}.",
	"{} takes a one way trip to Hades.",
	"{}, worried, looks down and sees their corpse.",
	"{} is carried off by the Valkyries.",
	"Let {}'s name be praised.",
	"{} no longer resides within the mortal realm.",
	"{} becomes the corpus delecti.",
	"{} is sent to the pearly gates by {}.",
	"{} is introduced to death by {}.",
	"{} now understands the agony of defeat.",
	"Nice try {}, but no cigar.",
	"{} becomes a part of history.",
	"{} suffers an untimely death at the hands of {}.",
	"{} was chopped into pieces by {}.",
	"The legend of {} was ended rather suddenly by {}",
	"{} is now a stain on {}'s weapon.",
	"{} is now an extension of {}'s weapon.",
	"The insects now have {} to themselves.",
	"{}'s corpse is now suffering at the hands of {}.",
	"{} floats around in the insides of {}",
	"{} expires due to {}'s blow.",
	"{} is now an ex-player... has ceased to be... is no more!",
	"{} wanders into the light with a helping hand from {}",
	"{} gets recycled by {}",
	"{} becomes a greasy smear on {}'s boot",
	"{} is now a greasy puddle.",
	"{}'s grave is danced on by {}, with great merriment",
	"{} has a hole in his stomach!",
	"{} finds themselves both diced AND sliced",
	"No guts, no glory. {}'s guts now belong to {}",
	"{} looks at the grass from below",
	"{} goes to look if god really exists",
	"{} goes the fast way to hell",
	"And there was much rejoicing as {} left the mortal coil",
	"{} was terminated with extreme predjudice",
	"{} simply expires",
	"{} was mortally wounded",
	"{} falls to the ground oozing",
	"Pause a moment and mourn the loss of {}",
	"All your {} are belong to {}.",
};

void remove_pvp(const dpp::snowflake id) {
	auto p1 = pvp_list.find(id);
	if (p1 != pvp_list.end()) {
		auto p2 = pvp_list.find(p1->second.opponent);
		if (p2 != pvp_list.end()) {
			pvp_list.erase(p2);
		}
		pvp_list.erase(p1);
	}
}

player get_pvp_opponent(const dpp::snowflake id, dpp::discord_client* shard) {
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
	pvp_list[event.command.usr.id] = {
		.opponent = opponent,
		.accepted = false,
		.my_turn = false,
	};
	pvp_list[opponent] = {
		.opponent = event.command.usr.id,
		.accepted = false,
		.my_turn = false,
	};
	player p2 = get_pvp_opponent(event.command.usr.id, event.from);
	send_chat(event.command.usr.id, p.paragraph, p2.name, "combat");
	dpp::message m = dpp::message("<@" + opponent.str() +  "> You have been challenged to combat by " + p.name).set_allowed_mentions(true, false, false, false, {}, {});
	m.channel_id = p2.event.command.channel_id;
	m.guild_id = p2.event.command.guild_id;
	m.add_component(
		dpp::component()
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("pvp_accept;" + event.command.usr.id.str() + ";" + opponent.str()))
			.set_label("Accept")
			.set_style(dpp::cos_success)
			.set_emoji(sprite::sword008.name, sprite::sword008.id)
		)
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("pvp_reject;" + event.command.usr.id.str() + ";" + opponent.str()))
			.set_label("Reject")
			.set_style(dpp::cos_danger)
			.set_emoji(sprite::magic05.name, sprite::magic05.id)
		)
	);
	
	//event.from->creator->message_create(m);
	p2.event.edit_original_response(m);
}

dpp::snowflake get_pvp_opponent_id(const dpp::snowflake id) {
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

void update_opponent_message(const dpp::interaction_create_t& event, const dpp::message& m) {
	if (has_active_pvp(event.command.usr.id)) {
		player p2 = get_pvp_opponent(event.command.usr.id, event.from);
		p2.event.edit_original_response(m);	
	}	
}

void accept_pvp(const dpp::snowflake id1, const dpp::snowflake id2) {
	bool turn = random(0, 1);
	pvp_list[id2] = {
		.opponent = id1,
		.accepted = true,
		.my_turn = turn,
	};
	pvp_list[id1] = {
		.opponent = id2,
		.accepted = true,
		.my_turn = turn,
	};
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
	auto p = pvp_list.find(id);
	return (p != pvp_list.end() && p->second.accepted == true);
}

bool is_my_pvp_turn(const dpp::snowflake id) {
	auto p = pvp_list.find(id);
	return (p != pvp_list.end() && p->second.accepted == true && p->second.my_turn == true);
}

dpp::message get_pvp_round(const dpp::interaction_create_t& event) {
	dpp::cluster& bot = *(event.from->creator);
	dpp::message m;
	component_builder cb(m);
	std::stringstream output;
	player opponent = get_pvp_opponent(event.command.usr.id, event.from);
	player p1 = get_live_player(event, false);

	output << "### " << p1.name << " vs " << opponent.name << "\n";

	output << "This section under construction. You probably shouldn't be here yet.";

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = "In PvP combat with " + opponent.name + ", Location: " + std::to_string(opponent.paragraph),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_description(output.str());
	
	m = cb.get_message();
	m.add_embed(embed);
	return m;
}

void continue_pvp_combat(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);

	dpp::message m(get_pvp_round(event));

	p.save(event.command.usr.id);
	update_live_player(event, p);

	event.reply(event.command.type == dpp::it_component_button ? dpp::ir_update_message : dpp::ir_channel_message_with_source, m.set_flags(dpp::m_ephemeral), [event, &bot, m, p](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying combat " + std::to_string(p.after_fragment) + " location " + std::to_string(p.paragraph) + ": " + cc.http_info.body);
			event.reply("Internal error displaying combat " + std::to_string(p.after_fragment) + " location " + std::to_string(p.paragraph) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
		}
	});
}

bool pvp_combat_nav(const dpp::button_click_t& event, player p, const std::vector<std::string>& parts) {
	if (!p.in_combat && !has_active_pvp(event.command.usr.id)) {
		return false;
	}
	bool claimed{false};

	if (parts[0] == "pvp_accept") {
		/* Fall-through to prevent going to PvE combat code after accept */
		claimed = true;
	}

	if (claimed) {
		continue_pvp_combat(event, p);
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


	output << "__Combat__: **" << p.name << "** vs. **" << p.combatant.name << "**\n\n";

	if (EStamina <= 0) {
		output << "This monster is already dead!\n\n";
		p.after_fragment++;
		p.combatant = {};
		p.in_combat = false;
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("follow_nav;" + std::to_string(p.paragraph) + ";" + std::to_string(p.paragraph)))
			.set_label("Continue")
			.set_style(dpp::cos_primary)
			.set_emoji("▶️", 0, false)	
		);
	} else {

		long EAttack = dice() + dice() + ESkill + EWeapon;
		long PAttack = dice() + dice() + p.skill + PWeapon;

		output << "Enemy Stance: **" << (EStance == DEFENSIVE ? "defensive 🛡️" : "offensive ⚔️") << "**\n";
		output << "Your Stance: **" << (p.stance == DEFENSIVE ? "defensive 🛡️" : "offensive ⚔️") << "**\n";

		if ((EStance == OFFENSIVE) && (p.stance == DEFENSIVE)) {
			int Bonus = dice();
			EAttack += Bonus;
			output << "You are shielding yourself from possible attack and the enemy is bearing down (+**" << Bonus << " to enemy attack score**).";
		} else  if ((p.stance == OFFENSIVE) && (EStance == DEFENSIVE)) {
			int Bonus = dice();
			PAttack += Bonus;
			output << "You are being offensive in stance, and the enemy is shielding themselves from your blow (**+" << Bonus << " to your attack score**).";
		}
	
		output << "\n\nYou get a total attack score of **" << PAttack << "**, the enemy gets a total attack score of **" << EAttack << "**.\n\n";

		long SaveRoll = dice() + dice();
		bool Saved = false;

		if (EAttack > PAttack) {
			output << "__**The enemy hits you**__.";
			
			if (p.stance == DEFENSIVE) {
				output << " Because you are in a defensive stance, you gain a bonus to your armour and are more able to defend against the blow.";
				SaveRoll -= dice();
			}
			if (SaveRoll <= PArmour) {
				Saved = true;
			}
		}
		else
		{
			output << "__**You hit the enemy**__.";
			
			if (EStance == DEFENSIVE) {
				output << " Because the enemy is cowering in a defensive position this round, it gains extra bonuses to its armour which may increase its chances of avoiding damage.";
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
			output << " The blow bounces harmlessly off armour...";
		} else {

			output << " The blow cuts through armour and lands in the **";
			switch (D6) {
				case 1:
					output << "head/neck";
					SDamage = dice();
					KDamage = 1;
					break;
				case 2:
					output << "legs/tail";
					SDamage = 3;
					KDamage = 1;
					break;
				case 3:
					output << "body/torso";
					SDamage = dice();
					KDamage = 0;
					break;
				case 4:
					output << "arms/other limbs";
					SDamage = 2;
					KDamage = 2;
					break;
				case 5:
					output << "hands/claws";
					SDamage = 2;
					KDamage = 1;
					break;
				case 6:
					output << "weapon";
					SDamage = 0;
					KDamage = 1;
					break;
			}

			output << "** area. ";


			switch (D6) {
				case 1:
					if (KAttackType == CUTTING) {
						output << "Because the attack was a cutting attack, it causes severe damage to this part of the body, and extra stamina points are lost as a result!";
						SDamage += dice();
					}
					break;
				case 2:
					if (KAttackType == PIERCING) {
						output << "Because the attack was a piercing attack, it causes severe damage to the body, and extra stamina is lost due to the attack!";
						SDamage += dice();
					}
					break;
			}

			output << " This causes ";

			if (SDamage == 0) {
				output << "no **stamina** loss, ";
			} else {
				output << SDamage << " points of **stamina** loss, ";
			}
			output << "and ";

			if (KDamage == 0) {
				output << "no **skill** loss. ";
			} else {
				output << KDamage << " points of **skill** loss. ";
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
				output << "You are very disorientated and confused and feeling very weak. ";
			} else if (EStamina < 4) {
				output << "The enemy is dazed and staggering, close to death. ";
			}

			if (p.skill < 5) {
				output << "Your hands are trembling and you are unable to properly aim at the enemy, ";
			} else if (ESkill < 5) {
				output << "The enemy is unable to focus properly upon you and stares at you trying to predict your next move. ";
			}

			if (EStamina < 1 || p.stamina < 1) {
				if (p.stamina < 1) {
					output << fmt::format(fmt::runtime(death_messages[random(0, death_messages.size() - 1)].data()), p.name, p.combatant.name);
				} else {
					/* Add experience on victory equal to remaining skill of enemy (indicates difficulty of the fight) */
					long xp = abs(std::min(p.combatant.skill, 0l) * 0.15f) + 1;
					output << "\n\n***+" + std::to_string(xp) + " experience points!***\n\n";
					p.add_experience(xp);
					output << "**" << fmt::format(fmt::runtime(death_messages[random(0, death_messages.size() - 1)].data()), p.combatant.name, p.name) << "**";
				}
			}
		}

		output << "\n\n```\n";
		output << fmt::format("{0:<30s}{1:<30s}", p.name, p.combatant.name) << "\n";
		output << fmt::format("{0:<30s}{1:<30s}", fmt::format("Skill: {0:2d}", p.skill), fmt::format("Skill: {0:2d}", p.combatant.skill)) << "\n";
		output << fmt::format("{0:<30s}{1:<30s}", fmt::format("Stamina: {0:2d}", p.stamina), fmt::format("Stamina: {0:2d}", p.combatant.stamina)) << "\n";
		output << fmt::format("{0:<30s}{1:<30s}", fmt::format("Armour: {0:2d}", p.armour.rating), fmt::format("Armour: {0:2d}", p.combatant.armour)) << "\n";
		output << fmt::format("{0:<30s}{1:<30s}", fmt::format("Weapon: {0:2d}", p.weapon.rating), fmt::format("Weapon: {0:2d}", p.combatant.weapon)) << "\n";
		output << "```\n\n";

		bool CombatEnded = false;
		if (p.stamina < 1) {
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("respawn"))
				.set_label("Respawn")
				.set_style(dpp::cos_danger)
				.set_emoji(sprite::skull.name, sprite::skull.id)
			);
			p.drop_everything();
			CombatEnded = true;
		} else if (EStamina < 1) {
			p.after_fragment++;
			p.combatant = {};
			p.in_combat = false;
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("follow_nav;" + std::to_string(p.paragraph) + ";" + std::to_string(p.paragraph)))
				.set_label("Continue")
				.set_style(dpp::cos_primary)
				.set_emoji("▶️", 0, false)	
			);
			CombatEnded = true;
		}

		if (!CombatEnded) {
			// todo: iterate inventory for anything with weapon score and present as option here,
			// also combat spells
			size_t index = 0;
			for (const auto & inv :  p.possessions) {
				if (inv.flags.length() >= 2 && inv.flags[0] == 'W' && isdigit(inv.flags[1])) {
					cb.add_component(dpp::component()
						.set_type(dpp::cot_button)
						.set_id(security::encrypt("attack;" + inv.name + ";" + inv.flags.substr(1, inv.flags.length() - 1) + ";" + std::to_string(++index)))
						.set_label("Attack using " + inv.name)
						.set_style(dpp::cos_secondary)
						.set_emoji("⚔️", 0, false)	
					);
				}
			}
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("change_stance;" + std::string(p.stance == DEFENSIVE ? "o" : "d")))
				.set_label("Stance: " + std::string(p.stance == DEFENSIVE ? "Defensive" : "Offensive") + " (click to change)")
				.set_style(dpp::cos_secondary)
				.set_emoji("🛡️", 0, false)
			);
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("change_strike;" + std::string(p.attack == CUTTING ? "p" : "c")))
				.set_label("Attack Type: " + std::string(p.attack == CUTTING ? "Cutting" : "Piercing") + " (click to change)")
				.set_style(dpp::cos_secondary)
				.set_emoji("🤺", 0, false)
			);
		}
	}
	
	cb.add_component(help_button());
	m = cb.get_message();

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = "In combat with " + p.combatant.name + ", Location " + std::to_string(p.paragraph),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_description(output.str());
	
	p.save(event.command.usr.id);
	update_live_player(event, p);
	m.add_embed(embed);

	event.reply(event.command.type == dpp::it_component_button ? dpp::ir_update_message : dpp::ir_channel_message_with_source, m.set_flags(dpp::m_ephemeral), [event, &bot, m, p](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying combat " + std::to_string(p.after_fragment) + " location " + std::to_string(p.paragraph) + ": " + cc.http_info.body);
			event.reply("Internal error displaying combat " + std::to_string(p.after_fragment) + " location " + std::to_string(p.paragraph) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
		}
	});

}
