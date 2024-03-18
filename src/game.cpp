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
#include <string>
#include <fmt/format.h>
#include <ssod/ssod.h>
#include <ssod/game.h>
#include <ssod/game_player.h>
#include <ssod/database.h>
#include <ssod/paragraph.h>
#include <ssod/game_util.h>
#include <ssod/component_builder.h>
#include <ssod/emojis.h>
#include <ssod/combat.h>
#include <ssod/aes.h>
#include <ssod/wildcard.h>
#include <ssod/inventory.h>

#define RESURRECT_SECS 3600
#define RESURRECT_SECS_PREMIUM 900

uint64_t get_guild_id(const player& p);

void send_chat(dpp::snowflake user_id, uint32_t paragraph, const std::string& message, const std::string& type, uint64_t guild_id) {
	if (guild_id) {
		db::query("INSERT INTO game_chat_events (event_type, user_id, location_id, message, guild_id) VALUES(?,?,?,?,?)",
			  {type, user_id, paragraph, message.substr(0, 140), guild_id});

	} else {
		db::query("INSERT INTO game_chat_events (event_type, user_id, location_id, message) VALUES(?,?,?,?)",
			  {type, user_id, paragraph, message.substr(0, 140)});
	}
}

void do_toasts(player &p, component_builder& cb) {
	/* Display and clear toasts */
	std::vector<std::string> toasts = p.get_toasts();
	for (const auto& toast : toasts) {
		dpp::embed e = dpp::embed()
			.set_colour(EMBED_COLOUR)
			.set_description(toast);
		if (toast.find("# You have met your end, adventurer") != std::string::npos) {
			e.set_image("https://images.ssod.org/resource/death.png");
		}
		cb.add_embed(e);
	}
}

void death(player& p, component_builder& cb) {
	p.stamina = 0;
	p.in_pvp_picker = false;
	std::string toast;
	time_t when = RESURRECT_SECS;

	toast += "# You have met your end, adventurer!\n\n";

	auto rs = db::query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { p.event.command.usr.id });
	if (!rs.empty()) {
		when = RESURRECT_SECS_PREMIUM;
	}

	if (p.last_resurrect == 0 || time(nullptr) > p.last_resurrect + when) {
		toast += "You can __resurrect nearby__ if you wish, which will respawn you __near to your place of death with your current inventory__.";
		toast += "If you choose this path, then perhaps in the future you will *choose more wisely*.\n\nPlease be aware that resurrections ";
		toast += "are limited to one per hour **or one per 15 minutes for premium users**, and if you have no resurrection available, and do ";
		toast += "not want to wait, your only choice is to __restart from the start__.";
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("resurrect"))
			.set_label("Ressurrect Me")
			.set_style(dpp::cos_success)
			.set_emoji(sprite::health_heart.name, sprite::health_heart.id)
		);
	} else {
		toast += "You __can't resurrect nearby__ until your resurrection is ready again " + dpp::utility::timestamp(p.last_resurrect + when, dpp::utility::tf_relative_time) + ", ";
		toast += "but you can respawn at the __start of the game__ right now and choose a different, *perhaps wiser* path...\n\n";
		if (rs.empty()) {
			toast += sprite::diamond.get_mention() + " ";
			toast += "[Premium users](https://premium.ssod.org/) can **__resurrect every 15 minutes__ instead of every hour.** If you had Seven Spells Premium, you could ";
			toast += "be back in the land of the living right now!";
		}
	}

	p.add_toast(toast);

	cb.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id(security::encrypt("respawn"))
		.set_label("Respawn at start")
		.set_style(dpp::cos_danger)
		.set_emoji(sprite::skull.name, sprite::skull.id)
	);
}

void add_chat(std::string& text, long paragraph_id, uint64_t guild_id) {
	text += "\n__**Chat**__\n```ansi\n";
	auto rs = db::query("SELECT *, TIME(sent) AS message_time FROM game_chat_events JOIN game_users ON game_chat_events.user_id = game_users.user_id WHERE sent > now() - 6000 AND (location_id = ? OR (guild_id = ? AND guild_id IS NOT NULL and event_type = 'chat')) ORDER BY sent DESC, id DESC LIMIT 5", {paragraph_id, guild_id});
	for (size_t x = 0; x < 5 - rs.size(); ++x) {
		text += std::string(80, ' ') + "\n";
	}
	if (!rs.empty()) {
		std::reverse(rs.begin(), rs.end());
	}
	for (const auto& row : rs) {
		if (row.at("event_type") == "chat") {
			if (row.at("location_id") == std::to_string(paragraph_id) || row.at("guild_id").empty()) {
				text += fmt::format("\033[2;31m[{}]\033[0m <\033[2;34m{}\033[0m> {}\n",
						    row.at("message_time"),
						    dpp::utility::markdown_escape(row.at("name")),
						    dpp::utility::markdown_escape(row.at("message")));
			} else if (row.at("guild_id") == std::to_string(guild_id)) {
				text += fmt::format("\033[2;31m[{}]\033[0m \033[2;35m[G]\033[0m <\033[2;34m{}\033[0m> {}\n",
						    row.at("message_time"),
						    dpp::utility::markdown_escape(row.at("name")),
						    dpp::utility::markdown_escape(row.at("message")));
			}
		} else if (row.at("event_type") == "join") {
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m wanders into the location\n", row.at("message_time"), dpp::utility::markdown_escape(row.at("name")));
		} else  if (row.at("event_type") == "part") {
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m leaves the location\n", row.at("message_time"), dpp::utility::markdown_escape(row.at("name")));
		} else  if (row.at("event_type") == "drop") {
			std::string item = row.at("message");
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m drops {} \033[2;34m{}\033[0m\n", row.at("message_time"), row.at("name"), std::string("aeiou").find(tolower(item[0])) != std::string::npos ? "an" : "a", dpp::utility::markdown_escape(item));
		} else  if (row.at("event_type") == "pickup") {
			std::string item = row.at("message");
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m picks up {} \033[2;34m{}\033[0m\n", row.at("message_time"), row.at("name"), std::string("aeiou").find(tolower(item[0])) != std::string::npos ? "an" : "a", dpp::utility::markdown_escape(item));
		} else  if (row.at("event_type") == "combat") {
			std::string item = row.at("message");
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m challenges \033[2;34m{}\033[0m to combat!\n", row.at("message_time"), dpp::utility::markdown_escape(row.at("name")), dpp::utility::markdown_escape(row.at("message")));
		} else  if (row.at("event_type") == "death") {
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m died{}...\n", row.at("message_time"), dpp::utility::markdown_escape(row.at("name")), row.at("message").empty() ? "" : " fighting  \033[2;34m" + dpp::utility::markdown_escape(row.at("message")) + "\033[0m");
		}
	}
	text += "```\n";
}

void game_input(const dpp::form_submit_t & event) {
	if (!player_is_live(event)) {
		return;
	}
	player p = get_live_player(event);
	dpp::cluster& bot = *(event.from->creator);
	bool claimed{true};
	std::string custom_id = security::decrypt(event.custom_id);
	if (custom_id.empty()) {
		return;
	}
	bot.log(dpp::ll_debug, std::to_string(event.command.usr.id) + ": " + custom_id);
	std::vector<std::string> parts = dpp::utility::tokenize(custom_id, ";");
	if (custom_id == "deposit_gold_amount_modal" && p.in_bank) {
		auto bank_amount = db::query("SELECT SUM(item_flags) AS gold FROM game_bank WHERE owner_id = ? AND item_desc = ?",{event.command.usr.id, "__GOLD__"});
		long balance_amount = atol(bank_amount[0].at("gold"));
		long amount = std::max(0l, atol(std::get<std::string>(event.components[0].components[0].value)));
		amount = std::min(amount, p.gold);
		amount = std::min(amount, p.max_gold() - balance_amount);
		if (p.gold > 0 && amount > 0) {
			p.add_gold(-amount);
			db::query("INSERT INTO game_bank (owner_id, item_desc, item_flags) VALUES(?,'__GOLD__',?)", {event.command.usr.id, amount});
		}
		claimed = true;
	} else if (parts[0] == "answer" && p.stamina > 0 && !p.in_bank && !p.in_inventory) {
		// id = "answer;" + std::to_string(n.paragraph) + ";" + n.prompt + ";" + n.answer + ";" + std::to_string(++unique);		
		std::string entered_answer = std::get<std::string>(event.components[0].components[0].value);
		if (dpp::lowercase(entered_answer) == dpp::lowercase(parts[3])) {
			p.after_fragment = 0; // Resets current combat index and announces travel
			p.challenged_by = 0ull;
			remove_pvp(event.command.usr.id);
			send_chat(event.command.usr.id, p.paragraph, "", "part");
			send_chat(event.command.usr.id, atoi(parts[1]), "", "join");
			p.paragraph = atol(parts[1]);
		} else {
			p.add_toast("### " + sprite::inv_drop.get_mention() + " The answer you entered is not correct, adventurer! Please try again!");
		}
		bot.log(dpp::ll_debug, "Answered: " + entered_answer);
		claimed = true;
	} else if (custom_id == "chat_modal" && p.stamina > 0) {
		std::string message = std::get<std::string>(event.components[0].components[0].value);
		uint64_t guild_id = get_guild_id(p);
		if (guild_id) {
			bot.log(dpp::ll_info, "Chat: [G(" + std::to_string(p.paragraph) + "," + std::to_string(guild_id) + ")] " + event.command.usr.id.str() + " <" + p.name + "> " + message);
			send_chat(event.command.usr.id, p.paragraph, message, "chat", guild_id);
		} else {
			bot.log(dpp::ll_info, "Chat: [L(" + std::to_string(p.paragraph) + ")] " + event.command.usr.id.str() + " <" + p.name + "> " + message);
			send_chat(event.command.usr.id, p.paragraph, message);
		}
		claimed = true;
	} else if (custom_id == "withdraw_gold_amount_modal" && p.in_bank) {
		long amount = std::max(0l, atol(std::get<std::string>(event.components[0].components[0].value)));
		auto bank_amount = db::query("SELECT SUM(item_flags) AS gold FROM game_bank WHERE owner_id = ? AND item_desc = ?",{event.command.usr.id, "__GOLD__"});
		long balance_amount = atol(bank_amount[0].at("gold"));
		/* Can't withdraw more than is in the bank, or more than you can carry */
		amount = std::min(amount, balance_amount);
		amount = std::min(amount, p.max_gold());
		if (balance_amount > 0 && amount > 0) {
			p.add_gold(amount);
			/* Coalesce gold in bank to one row */
			db::transaction();
			db::query("DELETE FROM game_bank WHERE owner_id = ? AND item_desc = '__GOLD__'", {event.command.usr.id});
			db::query("INSERT INTO game_bank (owner_id, item_desc, item_flags) VALUES(?,'__GOLD__',?)", {event.command.usr.id, balance_amount - amount});
			db::commit();
		}
	}
	if (claimed) {
		p.event = event;
		update_live_player(event, p);
		p.save(event.command.usr.id);
		continue_game(event, p);
	}
}

void game_select(const dpp::select_click_t &event) {
	if (!player_is_live(event)) {
		return;
	}
	bool claimed{false};
	player p = get_live_player(event);
	dpp::cluster& bot = *(event.from->creator);
	std::string custom_id = security::decrypt(event.custom_id);
	if (custom_id.empty()) {
		return;
	}
	bot.log(dpp::ll_debug, std::to_string(event.command.usr.id) + ": " + custom_id);
	if (custom_id == "withdraw" && p.in_bank && !event.values.empty()) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		db::transaction();
		auto rs = db::query("SELECT * FROM game_bank WHERE owner_id = ? AND item_desc = ? AND item_flags = ?", {event.command.usr.id, parts[0], parts[1]});
		if (!rs.empty()) {
			db::query("DELETE FROM game_bank WHERE owner_id = ? AND item_desc = ? AND item_flags = ?", {event.command.usr.id, parts[0], parts[1]});
			p.possessions.push_back(item{.name = parts[0], .flags = parts[1]});
			p.inv_change = true;
		}
		db::commit();
		claimed = true;
	} else if (custom_id == "deposit" && p.in_bank && !event.values.empty()) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		db::transaction();
		if (p.has_possession(parts[0])) {
			db::query("INSERT INTO game_bank (owner_id, item_desc, item_flags ) VALUES(?,?,?)", {event.command.usr.id, parts[0], parts[1]});
			p.drop_possession(item{.name = parts[0], .flags = parts[1]});
			p.inv_change = true;
		}
		db::commit();
		claimed = true;
	} else if (custom_id == "drop_item" && !event.values.empty() && p.in_inventory && p.stamina > 0) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		if (parts.size() >= 2 && p.has_possession(parts[0])) {
			sale_info si = get_sale_info(parts[0]);
			if (dpp::lowercase(parts[0]) != "scroll" && !si.quest_item) {
				/* Can't drop a scroll (is quest item) */
				p.drop_possession(item{.name = parts[0], .flags = parts[1]});
				if (p.armour.name == parts[0]) {
					p.armour.name = "Undergarments 👙";
					p.armour.rating = 0;
				} else if (p.weapon.name == parts[0]) {
					p.weapon.name = "Unarmed 👊";
					p.weapon.rating = 0;
				}
				/* Drop to floor */
				db::query(
					"INSERT INTO game_dropped_items (location_id, item_desc, item_flags) VALUES(?,?,?)",
					{p.paragraph, parts[0], parts[1]});
				send_chat(event.command.usr.id, p.paragraph, parts[0], "drop");
			}
		}
		claimed = true;
	} else if (custom_id == "use_item" && !event.values.empty() && p.in_inventory && p.stamina > 0) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		if (parts.size() >= 2 && p.has_possession(parts[0])) {
			p.drop_possession(item{.name = parts[0], .flags = parts[1]});
			std::string flags = parts[1];
			if (flags.substr(0, 2) == "ST") {
				long modifier = atol(flags.substr(2, flags.length() - 2));
				p.add_stamina(modifier);
			} else if (flags.substr(0, 2) == "SN") {
				long modifier = atol(flags.substr(2, flags.length() - 2));
				p.add_sneak(modifier);
			} else if (flags.substr(0, 2) == "SK") {
				long modifier = atol(flags.substr(2, flags.length() - 2));
				p.add_skill(modifier);
			} else if (flags.substr(0, 2) == "MA") {
				long modifier = atol(flags.substr(2, flags.length() - 2));
				p.add_mana(modifier);
			} else if (flags.substr(0, 2) == "SD") {
				long modifier = atol(flags.substr(2, flags.length() - 2));
				p.add_speed(modifier);
			} else if (flags.substr(0, 2) == "EX") {
				long modifier = atol(flags.substr(2, flags.length() - 2));
				p.add_experience(modifier);
			} else if (flags.substr(0, 2) == "LK") {
				long modifier = atol(flags.substr(2, flags.length() - 2));
				p.add_luck(modifier);
			} else if (flags.substr(0, 1) == "A") {
				long modifier = atol(flags.substr(1, flags.length() - 1));
				p.armour.rating += modifier;
			} else if (flags.substr(0, 1) == "W") {
				long modifier = atol(flags.substr(1, flags.length() - 1));
				p.weapon.rating += modifier;
			}
		}
		claimed = true;
	} else if (custom_id == "equip_item" && !event.values.empty() && p.in_inventory && p.stamina > 0) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		if (parts.size() >= 2 && p.has_possession(parts[0])) {
			if (parts[1][0] == 'W') {
				std::string rating = parts[1].substr(1, parts[1].length());
				p.weapon = rated_item{.name = parts[0], .rating = atol(rating)};
			} else {
				std::string rating = parts[1].substr(1, parts[1].length());
				p.armour = rated_item{.name = parts[0], .rating = atol(rating)};
			}
		}
		claimed = true;
	} else if (custom_id == "sell" && !p.in_inventory && !p.in_bank && !event.values.empty() && !p.in_combat) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		sale_info s = get_sale_info(parts[0]);
		if (p.has_possession(parts[0]) && s.sellable && !s.quest_item && dpp::lowercase(parts[0]) != "scroll") {
			if (p.armour.name == parts[0]) {
				p.armour.name = "Undergarments 👙";
				p.armour.rating = 0;
			} else if (p.weapon.name == parts[0]) {
				p.weapon.name = "Unarmed 👊";
				p.weapon.rating = 0;
			}
			p.drop_possession(item{.name = parts[0], .flags = parts[1]});
			p.gold += s.value;
			p.inv_change = true;
		}
		claimed = true;
	} else if (custom_id == "fight_pvp" && p.in_pvp_picker && !event.values.empty()) {
		dpp::snowflake user(event.values[0]);
		challenge_pvp(event, user);
		claimed = true;
	}
	if (claimed) {
		p.event = event;
		update_live_player(event, p);
		p.save(event.command.usr.id);
		continue_game(event, p);
	}
}

void game_nav(const dpp::button_click_t& event) {
	if (!player_is_live(event)) {
		return;
	}
	player p = get_live_player(event);
	bool claimed = false;
	if (p.state != state_play || event.custom_id.empty()) {
		return;
	}
	std::string custom_id = security::decrypt(event.custom_id);
	if (custom_id.empty()) {
		return;
	}
	event.from->log(dpp::ll_debug, std::to_string(event.command.usr.id) + ": " + custom_id);
	std::vector<std::string> parts = dpp::utility::tokenize(custom_id, ";");
	if (p.in_combat) {
		if (combat_nav(event, p, parts)) {
			return;
		}
	}
	if ((parts[0] == "follow_nav" || parts[0] == "follow_nav_pay" || parts[0] == "follow_nav_win") && parts.size() >= 3) {
		if (parts[0] == "follow_nav_pay" && parts.size() >= 4) {
			long link_cost = atol(parts[3]);
			if (p.gold < link_cost) {
				return;
			}
			p.gold -= link_cost;
		}
		if (parts[1] != parts[2]) {
			p.after_fragment = 0; // Resets current combat index and announces travel
			p.challenged_by = 0ull;
			remove_pvp(event.command.usr.id);
			send_chat(event.command.usr.id, atoi(parts[2]), "", "part");
			send_chat(event.command.usr.id, atoi(parts[1]), "", "join");
		}
		long dest = atol(parts[1]);
		if (paragraph::valid_next(p.paragraph, dest)) {
			p.paragraph = dest;
		}
		claimed = true;
	} else if (parts[0] == "shop" && parts.size() >= 6) {
		std::string flags = parts[3];
		long cost = atol(parts[4]);
		std::string name = parts[5];
		p.paragraph = atol(parts[1]);
		size_t max = p.max_inventory_slots();
		if (p.possessions.size() < max - 1) {
			if (p.gold >= cost) {
				if (flags == "SPELL") {
					p.gold -= cost;
					name = replace_string(name, " ", "");
					name = replace_string(name, "-", "");
					name = replace_string(name, ".", "");
					if (!p.has_spell(name)) {
						p.spells.push_back(item{.name = dpp::lowercase(name), .flags = flags});
					}
				} else if (flags == "HERB") {
					p.gold -= cost;
					if (!p.has_herb(name)) {
						p.herbs.push_back(item{.name = dpp::lowercase(name), .flags = flags});
					}
				} else {
					if (dpp::lowercase(name) == "scroll") {
						if (!p.has_flag("SCROLL", p.paragraph)) {
							p.gold -= cost;
							p.scrolls++;
							p.add_flag("SCROLL", p.paragraph);
						}
					} else {
						p.gold -= cost;
						item i{ .name = name, .flags = flags };
						if (!p.convert_rations(i)) {
							bool special{false};
							if (dpp::lowercase(name) == "horse" || dpp::lowercase(name) == "pack pony" || dpp::lowercase(name) == "donkey" || dpp::lowercase(name) == "mule") {
								if (!p.has_flag("horse")) {
									p.add_flag("horse");
									special = true;
								}
							} else if (dpp::lowercase(name) == "backpack" || dpp::lowercase(name) == "pack") {
								if (!p.has_flag("pack")) {
									p.add_flag("pack");
									special = true;
								}
							} else if (dpp::lowercase(name) == "saddle bags") {
								if (!p.has_flag("saddlebags")) {
									p.add_flag("saddlebags");
									special = true;
								}
							}
							if (!special) {
								p.possessions.push_back(i);
							}
						}
					}
				}
				p.inv_change = true;
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
			.armour = atol(parts[6]),
			.weapon = atol(parts[5]),
		};
		claimed = true;
	} else if (parts[0] == "bank" && !p.in_combat && !p.in_inventory) {
		p.in_bank = true;
		claimed = true;
	} else if (parts[0] == "answer" && !p.in_combat && !p.in_inventory) {
		dpp::interaction_modal_response modal(security::encrypt(custom_id), parts[2], {
			dpp::component()
			.set_label(parts[2])
			.set_id(security::encrypt("answer_prompt"))
			.set_type(dpp::cot_text)
			.set_min_length(1)
			.set_required(true)
			.set_max_length(64)
			.set_text_style(dpp::text_short)
		});
		event.dialog(modal);
		return;
	} else if (parts[0] == "deposit_gold" && p.in_bank) {
		dpp::interaction_modal_response modal(security::encrypt("deposit_gold_amount_modal"), "Deposit Gold",	{
			dpp::component()
			.set_label("Enter Gold Amount")
			.set_id(security::encrypt("deposit_gold_amount"))
			.set_type(dpp::cot_text)
			.set_placeholder("1")
			.set_min_length(1)
			.set_required(true)
			.set_max_length(64)
			.set_text_style(dpp::text_short)
		});
		event.dialog(modal);
		return;
	} else if (parts[0] == "withdraw_gold" && p.in_bank) {
		dpp::interaction_modal_response modal(security::encrypt("withdraw_gold_amount_modal"), "Withdraw Gold",	{
			dpp::component()
			.set_label("Enter Gold Amount")
			.set_id(security::encrypt("withdraw_gold_amount"))
			.set_type(dpp::cot_text)
			.set_placeholder("1")
			.set_min_length(1)
			.set_required(true)
			.set_max_length(64)
			.set_text_style(dpp::text_short)
		});
		event.dialog(modal);
		return;
	} else if (parts[0] == "pick_one" && parts.size() >= 5) {
		p.paragraph = atol(parts[1]);
		size_t max = p.max_inventory_slots();
		if (p.possessions.size() < max - 1) {
			if (!p.has_flag("PICKED", p.paragraph)) {
				item i{.name = parts[3], .flags = parts[4]};
				if (!p.convert_rations(i)) {
					p.possessions.push_back(i);
				}
				p.inv_change = true;
				p.add_flag("PICKED", p.paragraph);
			}
		}
		claimed = true;
	} else if (parts[0] == "respawn") {
		p.drop_everything();
		/* Load backup of player and save over the current */
		player new_p = player(event.command.usr.id, true);
		/* Keep experience points only (HARDCORE!!!) */
		new_p.experience = p.experience;
		new_p.in_combat = false;
		new_p.after_fragment = 0;
		new_p.combatant = {};
		new_p.possessions = {};
		new_p.breadcrumb_trail = {};
		new_p.state = state_play;
		new_p.gold = p.gold;
		new_p.silver = p.silver;
		new_p.notoriety = p.notoriety;
		new_p.last_resurrect = p.last_resurrect;
		new_p.death_xp_loss();
		new_p.reset_to_spawn_point();
		update_live_player(event, new_p);
		new_p.save(event.command.usr.id);
		p = new_p;
		claimed = true;
	} else if (parts[0] == "resurrect") {
		time_t when = RESURRECT_SECS;
		auto rs = db::query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { event.command.usr.id });
		if (!rs.empty()) {
			when = RESURRECT_SECS_PREMIUM;
		}
		if (time(nullptr) > p.last_resurrect + when) {
			/* Load backup of player and use its stamina and skill, respawn at the
			* start of its breadcrumb trail.
			*/
			player new_p = player(event.command.usr.id, true);
			p.in_combat = false;
			p.stamina = new_p.stamina;
			p.skill = new_p.skill;
			p.last_resurrect = time(nullptr);
			if (!p.breadcrumb_trail.empty()) {
				p.paragraph = p.breadcrumb_trail[0];
				p.breadcrumb_trail = { p.breadcrumb_trail[0] };
			} else {
				new_p.reset_to_spawn_point();
			}
			p.after_fragment = 0;
			p.combatant = {};
			p.state = state_play;
			update_live_player(event, p);
			p.save(event.command.usr.id);
			claimed = true;
		}
	} else if (parts[0] == "inventory" && parts.size() >= 2 && !p.in_combat && p.stamina > 0) {
		p.in_inventory = true;
		p.inventory_page = atoi(parts[1].c_str());
		claimed = true;
	} else if (parts[0] == "pick" && parts.size() >= 4 && !p.in_inventory && p.stamina > 0) {
		/* Pick up frm floor */
		long paragraph = atol(parts[1]);
		std::string name = parts[2];
		std::string flags = parts[3];
		size_t max = p.max_inventory_slots();
		if (p.possessions.size() < max - 1) {
			db::transaction();
			auto rs = db::query(
				"SELECT * FROM game_dropped_items WHERE location_id = ? AND item_desc = ? AND item_flags = ? LIMIT 1",
				{paragraph, name, flags});
			if (!rs.empty()) {
				db::query(
					"DELETE FROM game_dropped_items WHERE location_id = ? AND item_desc = ? AND item_flags = ? LIMIT 1",
					{paragraph, name, flags});
				item i{.name = name, .flags = flags};
				if (!p.convert_rations(i)) {
					p.possessions.push_back(i);
				}
				p.inv_change = true;
				send_chat(event.command.usr.id, p.paragraph, name, "pickup");
			}
			db::commit();
		}
		claimed = true;
	} else if (parts[0] == "exit_inventory" && parts.size() == 1 && !p.in_combat && p.stamina > 0) {
		p.in_inventory = false;
		claimed = true;
	} else if (parts[0] == "exit_bank" && parts.size() == 1 && !p.in_combat && p.stamina > 0) {
		p.in_bank = false;
		claimed = true;
	} else if (parts[0] == "refresh" && p.stamina > 0) {
		claimed = true;
	} else if (parts[0] == "pvp_picker" && p.stamina > 0) {
		p.in_pvp_picker = true;
		claimed = true;
	} else if (parts[0] == "exit_pvp_picker" && p.stamina > 0) {
		p.in_pvp_picker = false;
		claimed = true;
	} else if (parts[0] == "pvp_reject" && p.stamina > 0) {
		p.in_pvp_picker = false;
		player p2 = get_pvp_opponent(event.command.usr.id, event.from);
		dpp::snowflake opponent = get_pvp_opponent_id(event.command.usr.id);
		dpp::message m = dpp::message("<@" + opponent.str() +  "> " + p.name + " rejected your combat request.").set_allowed_mentions(true, false, false, false, {}, {});
		m.channel_id = p2.event.command.channel_id;
		m.guild_id = p2.event.command.guild_id;
		m.add_component(
			dpp::component()
			.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("exit_pvp_picker"))
				.set_label("Go Back")
				.set_style(dpp::cos_secondary)
				.set_emoji(sprite::magic05.name, sprite::magic05.id)
			)
		);
		p = end_pvp_combat(event);
		if (p2.event.from) {
			p2.event.edit_original_response(m);
		}
		claimed = true;
	} else if (parts[0] == "pvp_accept" && p.stamina > 0) {
		dpp::snowflake opponent = get_pvp_opponent_id(event.command.usr.id);
		accept_pvp(event.command.usr.id, opponent);
		player p2 = get_pvp_opponent(event.command.usr.id, event.from);
		p.in_pvp_picker = false;
		p = set_in_pvp_combat(event);
		update_opponent_message(event, get_pvp_round(p2.event), std::stringstream());
		claimed = true;
	} else if (parts[0] == "chat" && p.stamina > 0) {
		dpp::interaction_modal_response modal(security::encrypt("chat_modal"), "Chat",	{
			dpp::component()
			.set_label("Enter Message")
			.set_id(security::encrypt("chat_message"))
			.set_type(dpp::cot_text)
			.set_placeholder("hello")
			.set_min_length(1)
			.set_max_length(140)
			.set_required(true)
			.set_text_style(dpp::text_short)
		});
		event.dialog(modal);
		return;
	}
	if (claimed) {
		p.event = event;
		update_live_player(event, p);
		p.save(event.command.usr.id);
		continue_game(event, p);
	}
};

dpp::emoji get_emoji(const std::string& name, const std::string& flags) {
	dpp::emoji emoji = sprite::backpack;
	if (flags.length() && flags[0] == 'W') {
		if (dpp::lowercase(name).find("bow") != std::string::npos) {
			emoji = sprite::bow02;
		} else {
			emoji = sprite::sword008;
		}
	} else if (name.find("rrow") != std::string::npos) {
		emoji = sprite::bow08;
	} else if (name.find("scroll") != std::string::npos) {
		emoji = sprite::scroll02;
	} else if (name.find("key") != std::string::npos) {
		emoji = sprite::key01;
	} else if (name.find("ration") != std::string::npos) {
		emoji = sprite::cheese;
	} else if (name.find("food") != std::string::npos) {
		emoji = sprite::bread;
	} else if (flags.length() && flags[0] == 'A') {
		emoji = sprite::armor04;
	} else if (flags.substr(0, 3) == "ST+") {
		emoji = sprite::red03;
	} else if (flags.substr(0, 3) == "SK+") {
		emoji = sprite::green03;
	} else if (flags.substr(0, 3) == "LK+") {
		emoji = sprite::blue03;
	} else if (name.find("potion") != std::string::npos) {
		emoji = sprite::orange03;
	}
	return emoji;
}

void bank(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	std::stringstream content;

	auto bank_amount = db::query("SELECT SUM(item_flags) AS gold FROM game_bank WHERE owner_id = ? AND item_desc = ?",{event.command.usr.id, "__GOLD__"});
	long amount = atol(bank_amount[0].at("gold"));
	content << "__**Welcome to the Bank Of Utopia**__\n\n";
	content << "The bank can hold a maximum of **" << p.max_gold() << "** gold pieces. Level up to increase this limit.\n";
	content << "We will soon be offering competitive exchange rates for changing silver coins to gold. Check back later at a branch near you.\n";

	std::ranges::sort(p.possessions, [](const item &a, const item& b) -> bool { return a.name < b.name; });
	auto bank_items = db::query("SELECT * FROM game_bank WHERE owner_id = ? AND item_desc != ? ORDER BY item_desc LIMIT 25",{event.command.usr.id, "__GOLD__"});
	if (!bank_items.empty()) {
		content << "\n__**" << bank_items.size() << "/25 Bank Items**__\n";
		for (const auto& bank_item : bank_items) {
			content << sprite::backpack.get_mention() << " " << bank_item.at("item_desc") << " - *" << describe_item(bank_item.at("item_flags"), bank_item.at("item_desc")) << "*\n";
		}
	}

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = "Bank Of Utopia",
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.add_field("Your Balance", std::to_string(amount) + " Gold " + sprite::gold_coin.get_mention(), true)
		.add_field("Coin Purse", std::to_string(p.gold) + " Gold " + sprite::gold_coin.get_mention(), true)
		.set_colour(EMBED_COLOUR)
		.set_description(content.str());
	
	dpp::message m;
	dpp::component deposit_menu, withdraw_menu;

	deposit_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder("Deposit Item")
		.set_id(security::encrypt("deposit"));
	size_t index{0};
	std::set<std::string> ds;
	for (const auto& inv : p.possessions) {
		if (dpp::lowercase(inv.name) == "scroll") {
			/* Can't bank a scroll! */
			continue;
		}
		if (ds.find(inv.name) == ds.end()) {
			dpp::emoji e = get_emoji(inv.name, inv.flags);
			if (deposit_menu.options.size() < 25) {
				deposit_menu.add_select_option(
					dpp::select_option(inv.name, inv.name + ";" + inv.flags, describe_item(inv.flags, inv.name).substr(0, 100))
					.set_emoji(e.name, e.id)
				);
				ds.insert(inv.name);
				index++;
			}
		}
	}
	withdraw_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder("Withdraw Item")
		.set_id(security::encrypt("withdraw"));
	std::set<std::string> dup_set;
	for (const auto& bank_item : bank_items) {
		if (dup_set.find(bank_item.at("item_desc")) == dup_set.end()) {
			dpp::emoji e = get_emoji(bank_item.at("item_desc"), bank_item.at("item_flags"));
			if (withdraw_menu.options.size() < 25) {
				withdraw_menu.add_select_option(
					dpp::select_option(bank_item.at("item_desc"), bank_item.at("item_desc") + ";" + bank_item.at("item_flags"), describe_item(bank_item.at("item_flags"), bank_item.at("item_desc")).substr(0, 100))
					.set_emoji(e.name, e.id)
				);
				dup_set.insert(bank_item.at("item_desc"));
			}
		}
	}

	m.add_embed(embed);
	if (dup_set.size() < 25 && index > 0) {
		/* User has something they can deposit in their inventory and bank is not full */
		m.add_component(dpp::component().add_component(deposit_menu));
	}
	if (withdraw_menu.options.size() > 0) {
		/* Bank has items which can be withdrawn */
		m.add_component(dpp::component().add_component(withdraw_menu));
	}
	m.add_component(
		dpp::component()
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("exit_bank"))
			.set_label("Back")
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::magic05.name, sprite::magic05.id)
		)
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("deposit_gold"))
			.set_label("Deposit Gold")
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::gold_coin.name, sprite::gold_coin.id)
			.set_disabled(p.gold == 0)
		)
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("withdraw_gold"))
			.set_label("Withdraw Gold")
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::gold_coin.name, sprite::gold_coin.id)
			.set_disabled(amount == 0)
		)
		.add_component(help_button())
	);


	event.reply(event.command.type == dpp::it_application_command ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying bank: " + cc.http_info.body + " Message: " + m.build_json());
			event.reply(dpp::message("Internal error displaying bank:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```").set_flags(dpp::m_ephemeral));
		}
	});
}

void pvp_picker(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	std::stringstream content;

	int64_t t = time(nullptr) - 600;
	auto others = db::query("SELECT * FROM game_users WHERE lastuse > ? AND paragraph = ? AND user_id != ? ORDER BY lastuse DESC LIMIT 25", {t, p.paragraph, event.command.usr.id});
	dpp::snowflake opponent_id = get_pvp_opponent_id(event.command.usr.id);

	if (opponent_id.empty()) {
		content << "\n__**Select player to fight**__\n\nThe player you pick will be sent a request and must accept it to fight you. If they leave the area, the request will be cancelled.\n";
	} else {
		content << "\n__**Please wait for your challenge to be accepted**__\n\nIf the user does not accept or reject your request, you will receive no response. If you wish to give up waiting, simply click the " + sprite::magic05.get_mention() + " Back button.\n";
	}

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = "PvP Player Selection for " + p.name,
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(content.str());
	
	dpp::message m;
	dpp::component fight_menu;

	if (others.size() > 0 && opponent_id.empty()) {

		fight_menu.set_type(dpp::cot_selectmenu)
			.set_min_values(0)
			.set_max_values(1)
			.set_placeholder("Select Player")
			.set_id(security::encrypt("fight_pvp"));
		for (const auto& other: others) {
			fight_menu.add_select_option(
				dpp::select_option(other.at("name"), other.at("user_id"), "Stamina: " + other.at("stamina") + ", Skill: " + other.at("skill") + ", " + other.at("experience") + " XP")
			);
		}
	} else {
		content << "\nThere are no players here to fight. You must go back.";
	}

	m.add_embed(embed);

	if (!opponent_id) {
		if (others.size() > 0) {
			m.add_component(dpp::component().add_component(fight_menu));
		}
	}

	m.add_component(
		dpp::component()
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("exit_pvp_picker"))
			.set_label("Back")
			.set_style(dpp::cos_secondary)
			.set_emoji(sprite::magic05.name, sprite::magic05.id)
		)
		.add_component(help_button())
	);

	event.reply(event.command.type == dpp::it_application_command || event.command.type == dpp::it_component_button ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, cc.http_info.body);
			event.reply(dpp::message("The player you have selected has is not active at this location! Perhaps they left the area while you were choosing?").add_component(
				dpp::component()
				.add_component(dpp::component()
				       .set_type(dpp::cot_button)
				       .set_id(security::encrypt("exit_pvp_picker"))
				       .set_label("Back")
				       .set_style(dpp::cos_secondary)
				       .set_emoji(sprite::magic05.name, sprite::magic05.id)
				)
				.add_component(help_button())
			)
			.add_component(help_button()).set_flags(dpp::m_ephemeral));
		}
	});
}

uint64_t get_guild_id(const player& p) {
	auto rs = db::query("SELECT guild_id FROM guild_members WHERE user_id = ?", { p.event.command.usr.id });
	if (!rs.empty()) {
		return atoll(rs[0].at("guild_id"));
	}
	return 0;
}

void continue_game(const dpp::interaction_create_t& event, player p) {
	if (p.in_combat) {
		if (has_active_pvp(event.command.usr.id)) {
			continue_pvp_combat(event, p, std::stringstream());
		} else {
			continue_combat(event, p);
		}
		return;
	} else if (p.in_pvp_picker) {
		pvp_picker(event, p);
		return;
	} else if (p.in_inventory) {
		inventory(event, p);
		return;
	} else if (p.in_bank) {
		bank(event, p);
		return;
	}
	paragraph location(p.paragraph, p, event.command.usr.id);
	/* If the current paragraph is an empty page with nothing but a link, skip over it.
	 * These link pages are old data and not relavent to gameplay. Basically just a paragraph
	 * that says "Turn to X" which were an anti-cheat holdover from book-form content.
	 */
	while (location.words == 0 && location.navigation_links.size() > 0 && (location.navigation_links[0].type == nav_type_autolink || location.navigation_links[0].type == nav_type_link)) {
		location = paragraph(location.navigation_links[0].paragraph, p, event.command.usr.id);
		p.paragraph = location.id;
	}
	dpp::cluster& bot = *(event.from->creator);
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = "Location " + location.secure_id,
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(location.text);
	dpp::message m;
	std::array<std::string, 3> types{".png", ".jpg", ".webm"};
	for (const std::string& ext : types) {
		if (fs::exists("../resource/paragraph_pictures/" + std::to_string(location.id) + ext)) {
			embed.set_image("https://images.ssod.org/resource/paragraph_pictures/" + std::to_string(location.id) + ext);
			break;
		}
	}

	m.add_embed(embed);
	int64_t t = time(nullptr) - 600;
	auto others = db::query("SELECT * FROM game_users WHERE lastuse > ? AND paragraph = ? AND user_id != ? ORDER BY lastuse DESC LIMIT 25", {t, p.paragraph, event.command.usr.id});
	if (others.size() > 0 || location.dropped_items.size() > 0) {
		std::string list_others, list_dropped, text;
		for (const auto & other : others) {
			list_others += dpp::utility::markdown_escape(other.at("name"), true) + ", ";
		}
		if (list_others.length()) {
			list_others = list_others.substr(0, list_others.length() - 2);
			text += "**__Other players here:__**\n" + list_others + "\n\n";
		}
		if (location.dropped_items.size()) {
			for (const auto & dropped : location.dropped_items) {
				list_dropped += dpp::utility::markdown_escape(dropped.name, true);
				if (dropped.qty > 1) {
					list_dropped += " (x " + std::to_string(dropped.qty) + ")";
				}
				list_dropped += ", ";
			}
			if (list_dropped.length()) {
				list_dropped = list_dropped.substr(0, list_dropped.length() - 2);
				text += "**__Items On Ground__**\n" + list_dropped + "\n\n";
			}
		}
		add_chat(text, location.id, get_guild_id(p));
		m.add_embed(dpp::embed()
			.set_colour(EMBED_COLOUR)
			.set_description(text)
		);
	} else {
		std::string text = "";
		add_chat(text, location.id, get_guild_id(p));
		m.add_embed(dpp::embed()
			.set_colour(EMBED_COLOUR)
			.set_description(text)
		);
	}

	component_builder cb(m);
	size_t index{0}, enabled_links{0};
	bool respawn_button_shown{false};
	size_t unique{0};

	if (location.trader) {
		/* Can sell items here */
		dpp::component sell_menu;

		sell_menu.set_type(dpp::cot_selectmenu)
			.set_min_values(0)
			.set_max_values(1)
			.set_placeholder("Sell Item")
			.set_id(security::encrypt("sell"));
		size_t index2{0};
		std::set<std::string> ds;
		for (const auto& inv : p.possessions) {
			sale_info s = get_sale_info(inv.name);
			if (!s.sellable || s.quest_item || dpp::lowercase(inv.name) == "scroll") {
				continue;
			}
			if (ds.find(inv.name) == ds.end()) {
				dpp::emoji e = get_emoji(inv.name, inv.flags);
				if (sell_menu.options.size() < 25) {
					sell_menu.add_select_option(
						dpp::select_option(inv.name, inv.name + ";" + inv.flags, "Value: " + std::to_string(s.value) + " - " + describe_item(inv.flags, inv.name).substr(0, 80))
							.set_emoji(e.name, e.id)
					);
					ds.insert(inv.name);
					index2++;
				}
			}
		}

		if (!sell_menu.options.empty()) {
			cb.add_menu(sell_menu);
		}
	}

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
				id = "follow_nav_pay;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::to_string(n.cost) + ";" + std::to_string(++unique);
				enabled_links++;
				break;
			case nav_type_pick_one:
				// PICKED
				label = "Choose " + n.buyable.name;
				id = "pick_one;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + n.buyable.name + ";" + n.buyable.flags + ";" + std::to_string(++unique);
				enabled_links++;
				break;
			case nav_type_shop:
				label = "Buy " + n.buyable.name + " (" + std::to_string(n.cost) + " Gold)";
				id = "shop;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::string(n.buyable.flags) + ";" + std::to_string(n.cost) + ";" + n.buyable.name + ";" + std::to_string(++unique);
				if (p.has_herb(n.buyable.name) || p.has_spell(n.buyable.name) || p.gold < n.cost) {
					comp.set_disabled(true);
				} else {
					enabled_links++;
				}
				break;
			case nav_type_bank:
				label = "Use Bank";
				id = "bank;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::to_string(++unique);
				enabled_links++;
				break;
			case nav_type_combat:
				label = "Fight " + n.monster.name;
				id = "combat;" + std::to_string(n.paragraph) + ";" + n.monster.name + ";" + std::to_string(n.monster.stamina) + ";" + std::to_string(n.monster.skill) + ";" + std::to_string(n.monster.armour) + ";" + std::to_string(n.monster.weapon) + ";" + std::to_string(++unique);
				enabled_links++;
				break;
			case nav_type_respawn:
				death(p, cb);
				p.save(event.command.usr.id);
				update_live_player(event, p);
				respawn_button_shown = true;
				break;
			case nav_type_modal:
				label = "Answer: " + n.prompt;
				id = "answer;" + std::to_string(n.paragraph) + ";" + n.prompt + ";" + n.answer + ";" + std::to_string(++unique);
				enabled_links++;
				break;
			default:
				id = "follow_nav;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::to_string(++unique);
				label = n.label;
				enabled_links++;
				break;
		}
		comp.set_type(dpp::cot_button)
			.set_id(security::encrypt(id))
			.set_label(label)
			.set_style(n.type == nav_type_combat ? dpp::cos_danger : dpp::cos_primary)
			.set_emoji(directions[++index], 0, false);

		if (n.type == nav_type_respawn) {
			comp.set_emoji(sprite::skull.name, sprite::skull.id);
		} else if (n.type == nav_type_bank) {
			comp.set_emoji(sprite::gold_bar.name, sprite::gold_bar.id);
		} else if (n.type == nav_type_modal) {
			comp.set_emoji("❓");
		}

		if (n.type != nav_type_respawn) {
			cb.add_component(comp);
		}
	}
	if (enabled_links == 0 && !respawn_button_shown) {
		death(p, cb);
	}

	do_toasts(p, cb);

	p.save(event.command.usr.id);
	update_live_player(event, p);

	if (enabled_links > 0 && p.stamina > 0) {
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("inventory;0"))
			.set_label("Inventory")
			.set_style(dpp::cos_secondary)
			.set_emoji(sprite::backpack.name, sprite::backpack.id)
		);
	}

	if (enabled_links > 0 && p.stamina > 0 && p.after_fragment == 0) {
		if (!others.empty()) {
			/* Can fight other players, present option */
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("pvp_picker"))
				.set_label("PvP")
				.set_style(dpp::cos_secondary)
				.set_emoji(sprite::bow09.name, sprite::bow09.id)
			);
		}

		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("refresh"))
			.set_style(dpp::cos_secondary)
			.set_emoji("♻")
		);

		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("chat"))
			.set_style(dpp::cos_secondary)
			.set_emoji("💬")
		);

		for (const auto & dropped : location.dropped_items) {
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("pick;" + std::to_string(p.paragraph) + ";" + dropped.name + ";" + dropped.flags))
				.set_label("Pick up " + dropped.name)
				.set_style(dpp::cos_secondary)
				.set_emoji(sprite::backpack.name, sprite::backpack.id)
			);
		}
	}

	cb.add_component(help_button());
	m = cb.get_message();

	event.reply(event.command.type == dpp::it_component_button ? dpp::ir_update_message : dpp::ir_channel_message_with_source, m.set_flags(dpp::m_ephemeral), [event, &bot, location, m](const auto& cc) {
		if (cc.is_error()) {{
			bot.log(dpp::ll_error, "Internal error displaying location " + std::to_string(location.id) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
			event.reply(dpp::message("Internal error displaying location " + std::to_string(location.id) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```").set_flags(dpp::m_ephemeral));
		}}
	});
}
