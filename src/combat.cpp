#include <dpp/dpp.h>
#include <ssod/combat.h>
#include <ssod/component_builder.h>
#include <ssod/game_util.h>
#include <ssod/game_dice.h>
#include <ssod/database.h>
#include <ssod/ssod.h>
#include <fmt/format.h>

constexpr const char* death_messages[48]{
	"%s becomes another greasy stain on the desert sands.",
	"%s departs the land of the living.",
	"%s travels to the next world courtesy of %s.",
	"%s lead a heroic, but brief life.",
	"The life of %s was cut short by %s.",
	"%s is now dancing with the reaper.",
	"%s wins a free trip across the styx from %s.",
	"%s takes a one way trip to Hades.",
	"%s, worried, looks down and sees their corpse.",
	"%s is carried off by the Valkyries.",
	"Let %s's name be praised.",
	"%s no longer resides within the mortal realm.",
	"%s becomes the corpus delecti.",
	"%s is sent to the pearly gates by %s.",
	"%s is introduced to death by %s.",
	"%s now understands the agony of defeat.",
	"Nice try %s, but no cigar.",
	"%s becomes a part of history.",
	"%s suffers an untimely death at the hands of %s.",
	"%s gets assrammed with a large spikey stick by %s.",
	"%s was chopped into pieces by %s.",
	"The legend of %s was ended rather suddenly by %s",
	"%s is now a stain on %s's weapon.",
	"%s is now an extension of %s's weapon.",
	"The insects now have %s to themselves.",
	"%s's corpse is now suffering at the hands of %s.",
	"%s floats around in the insides of %s",
	"%s expires due to %s's blow.",
	"%s died from an unknown disease from the weapon of %s",
	"%s is now an ex-player... has ceased to be... is no more!",
	"%s wanders into the light with a helping hand from %s",
	"%s gets recycled by %s",
	"%s becomes a greasy smear on %s's boot",
	"%s is now a greasy puddle.",
	"%s's grave is danced on by %s, with great merriment",
	"%s has a hole in his stomach!",
	"%s finds themselves both diced AND sliced",
	"No guts, no glory. %s's guts now belong to %s",
	"%s looks at the grass from below",
	"%s goes to look if god really exists",
	"%s goes the fast way to hell",
	"And there was much rejoicing as %s left the mortal coil",
	"%s was terminated with extreme predjudice",
	"%s simply expires",
	"%s was mortally wounded",
	"%s falls to the ground oozing",
	"Pause a moment and mourn the loss of %s",
	"All your %s are belong to %s.",
};

bool combat_nav(const dpp::button_click_t& event, player p, const std::vector<std::string>& parts) {
	if (!p.in_combat) {
		return false;
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
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id("follow_nav;" + std::to_string(p.paragraph) + ";" + std::to_string(p.paragraph))
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
				size_t limit = p.combatant.name.length() + p.name.length() + 255;
				char dm[limit];
				if (p.stamina < 1) {
					snprintf(dm, limit - 1, death_messages[rand() % 48], p.name.c_str(), p.combatant.name.c_str());
				} else {
					snprintf(dm, limit - 1, death_messages[rand() % 48], p.combatant.name.c_str(), p.name.c_str());
					/* Add experience on victory equal to remaining skill of enemy (indicates difficulty of the fight) */
					long xp = abs(std::min(p.combatant.skill, 0l) * 0.15f) + 1;
					output << "\n\n***+" + std::to_string(xp) + " experience points!***\n\n";
					p.add_experience(xp);
				}
				output << "**" << dm << "**";
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
				.set_id("respawn")
				.set_label("Respawn")
				.set_style(dpp::cos_danger)
				.set_emoji("💀", 0, false)	
			);
			CombatEnded = true;
		} else if (EStamina < 1) {
			p.after_fragment++;
			p.combatant = {};
			p.in_combat = false;
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id("follow_nav;" + std::to_string(p.paragraph) + ";" + std::to_string(p.paragraph))
				.set_label("Continue")
				.set_style(dpp::cos_primary)
				.set_emoji("▶️", 0, false)	
			);
			CombatEnded = true;
		}

		if (!CombatEnded) {
			// todo: iterate inventory for anything with weapon score and present as option here,
			// also combat spells
			for (const auto & inv :  p.possessions) {
				if (inv.flags.length() >= 2 && inv.flags[0] == 'W' && isdigit(inv.flags[1])) {
					cb.add_component(dpp::component()
						.set_type(dpp::cot_button)
						.set_id("attack;" + inv.name + ";" + inv.flags.substr(1, inv.flags.length() - 1))
						.set_label("Attack using " + inv.name)
						.set_style(dpp::cos_secondary)
						.set_emoji("⚔️", 0, false)	
					);
				}
			}
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id("change_stance;" + std::string(p.stance == DEFENSIVE ? "o" : "d"))
				.set_label("Stance: " + std::string(p.stance == DEFENSIVE ? "Defensive" : "Offensive") + " (click to change)")
				.set_style(dpp::cos_secondary)
				.set_emoji("🛡️", 0, false)
			);
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id("change_strike;" + std::string(p.attack == CUTTING ? "p" : "c"))
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