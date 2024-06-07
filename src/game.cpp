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
#include <regex>
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
#include <ssod/regex.h>
#include <ssod/grimoire.h>
#include <ssod/campfire.h>
#include <ssod/game_dice.h>

using namespace i18n;

#define RESURRECT_SECS 3600
#define RESURRECT_SECS_PREMIUM 900

uint64_t get_guild_id(const player& p);

void send_chat(dpp::snowflake user_id, uint32_t paragraph, const std::string& message, const std::string& type, uint64_t guild_id) {
	if (guild_id) {
		db::query("INSERT INTO game_chat_events (event_type, user_id, location_id, message, guild_id) VALUES(?,?,?,?,?)",
			  {type, user_id, paragraph, dpp::utility::utf8substr(message, 0, 140), guild_id});

	} else {
		db::query("INSERT INTO game_chat_events (event_type, user_id, location_id, message) VALUES(?,?,?,?)",
			  {type, user_id, paragraph, dpp::utility::utf8substr(message, 0, 140)});
	}
}

void do_toasts(player &p, component_builder& cb) {
	/* Display and clear toasts */
	std::vector<toast> toasts = p.get_toasts();
	for (const auto& toast : toasts) {
		dpp::embed e = dpp::embed()
			.set_colour(EMBED_COLOUR)
			.set_description(toast.message);
		if (!toast.image.empty()) {
			if (toast.image == "death.png") {
				e.set_image("https://images.ssod.org/resource/" + toast.image);
			} else {
				e.set_thumbnail("https://images.ssod.org/resource/" + toast.image);
			}
		}
		cb.add_embed(e);
	}
}

void death(player& p, component_builder& cb) {
	p.stamina = 0;
	p.in_pvp_picker = false;
	std::string toast;
	time_t when = RESURRECT_SECS;

	toast += "# " + tr("DED", p.event) + "\n\n";

	if (!p.event.command.entitlements.empty()) {
		when = RESURRECT_SECS_PREMIUM;
	} else {
		auto rs = db::query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", {p.event.command.usr.id});
		if (!rs.empty()) {
			when = RESURRECT_SECS_PREMIUM;
		}
	}

	if (p.last_resurrect == 0 || time(nullptr) > p.last_resurrect + when) {
		toast += tr("DEATH_EXPLAIN", p.event);
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("resurrect"))
			.set_label(tr("RESURRECT_ME", p.event))
			.set_style(dpp::cos_success)
			.set_emoji(sprite::health_heart.name, sprite::health_heart.id)
		);
	} else {
		toast += tr("RESURRECT_NOT_AVAILABLE", p.event, dpp::utility::timestamp(p.last_resurrect + when, dpp::utility::tf_relative_time));
		if (when == RESURRECT_SECS_PREMIUM) {
			toast += sprite::diamond.get_mention() + " ";
			toast += tr("PREMIUM_RESURRECT", p.event);
		}
	}

	p.add_toast({ .message = toast, .image = "death.png" });

	cb.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id(security::encrypt("respawn"))
		.set_label(tr("RESPAWN_AT_START", p.event))
		.set_style(dpp::cos_danger)
		.set_emoji(sprite::skull.name, sprite::skull.id)
	);

	cb.add_component(dpp::component()
		 .set_type(dpp::cot_button)
		 .set_id(security::encrypt("vote-topgg"))
		 .set_label(tr("TOPGG_VOTE", p.event))
		 .set_url("https://top.gg/bot/620654573547159553/vote")
		 .set_style(dpp::cos_link)
	);

	cb.add_component(dpp::component()
		 .set_type(dpp::cot_button)
		 .set_id(security::encrypt("vote-dbl"))
		 .set_label(tr("DBL_VOTE", p.event))
		 .set_url("https://discordbotlist.com/bots/the-seven-spells-of-destruction/upvote")
		 .set_style(dpp::cos_link)
	);

}

void add_chat(std::string& text, const dpp::interaction_create_t& event, long paragraph_id, uint64_t guild_id) {
	text += "\n__**" + tr("CHAT", event) + "**__\n```ansi\n";
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
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m {}\n", row.at("message_time"), dpp::utility::markdown_escape(row.at("name")), tr("WANDERS_IN", event));
		} else  if (row.at("event_type") == "part") {
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m {}\n", row.at("message_time"), dpp::utility::markdown_escape(row.at("name")), tr("LEAVES", event));
		} else  if (row.at("event_type") == "drop") {
			std::string item = row.at("message");
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m {} {} \033[2;34m{}\033[0m\n", row.at("message_time"), row.at("name"), tr("DROPS", event), std::string("aeiou").find(tolower(item[0])) != std::string::npos ? "an" : "a", dpp::utility::markdown_escape(item));
		} else  if (row.at("event_type") == "pickup") {
			std::string item = row.at("message");
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m picks up {} \033[2;34m{}\033[0m\n", row.at("message_time"), row.at("name"), std::string("aeiou").find(tolower(item[0])) != std::string::npos ? "an" : "a", dpp::utility::markdown_escape(item));
		} else  if (row.at("event_type") == "combat") {
			std::string item = row.at("message");
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m {} \033[2;34m{}\033[0m to combat!\n", row.at("message_time"), dpp::utility::markdown_escape(row.at("name")), tr("CHALLENGES", event), dpp::utility::markdown_escape(row.at("message")), tr("TO_COMBAT", event));
		} else  if (row.at("event_type") == "death") {
			text += fmt::format("\033[2;31m[{}]\033[0m *** \033[2;34m{}\033[0m {}{}...\n", row.at("message_time"), dpp::utility::markdown_escape(row.at("name")), tr("DIED", event), row.at("message").empty() ? "" : tr("FIGHTING", event) + " \033[2;34m" + dpp::utility::markdown_escape(row.at("message")) + "\033[0m");
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
	bot.log(dpp::ll_debug, event.command.locale + " " + std::to_string(event.command.usr.id) + ": " + custom_id);
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
			p.add_toast({ .message = "### " + sprite::inv_drop.get_mention() + " " + tr("INCORRECT_RIDDLE", event), .image = "confused.png" });
		}
		bot.log(dpp::ll_debug, "Answered: " + entered_answer);
		claimed = true;
	} else if (custom_id == "chat_modal" && p.stamina > 0) {
		std::string message = std::get<std::string>(event.components[0].components[0].value);
		uint64_t guild_id = get_guild_id(p);
		if (guild_id) {
			bot.log(dpp::ll_info, p.event.command.locale + " " + " Chat: [G(" + std::to_string(p.paragraph) + "," + std::to_string(guild_id) + ")] " + event.command.usr.id.str() + " <" + p.name + "> " + message);
			send_chat(event.command.usr.id, p.paragraph, message, "chat", guild_id);
		} else {
			bot.log(dpp::ll_info, p.event.command.locale + " " + " Chat: [L(" + std::to_string(p.paragraph) + ")] " + event.command.usr.id.str() + " <" + p.name + "> " + message);
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
	bot.log(dpp::ll_debug, event.command.locale + " " + std::to_string(event.command.usr.id) + ": " + custom_id);
	if (custom_id == "withdraw" && p.in_bank && !event.values.empty()) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		db::transaction();
		auto rs = db::query("SELECT * FROM game_bank WHERE owner_id = ? AND item_desc = ? AND item_flags = ? LIMIT 1", {event.command.usr.id, parts[0], parts[1]});
		if (!rs.empty()) {
			db::query("DELETE FROM game_bank WHERE id = ?", { rs[0].at("id") });
			p.pickup_possession(stacked_item{.name = parts[0], .flags = parts[1], .qty = 1 });
			p.inv_change = true;
		}
		db::commit();
		claimed = true;
	} else if (custom_id == "deposit" && p.in_bank && !event.values.empty()) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		db::transaction();
		if (p.has_possession(parts[0])) {
			db::query("INSERT INTO game_bank (owner_id, item_desc, item_flags) VALUES(?,?,?)", {event.command.usr.id, parts[0], parts[1]});
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
					p.armour.name = tr("NO_ARMOUR", event) + " 👙";
					p.armour.rating = 0;
				} else if (p.weapon.name == parts[0]) {
					p.weapon.name = tr("NO_WEAPON", event) + " 👊";
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
	} else if (custom_id == "cast" && !event.values.empty() && p.in_grimoire && p.stamina > 0) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		spell_info si = get_spell_info(parts[0]);
		if (parts.size() >= 1 && p.has_spell(parts[0]) && p.has_component_herb(parts[0]) && p.mana >= si.mana_cost) {
			p.add_mana(-si.mana_cost);
			auto effect = db::query("SELECT * FROM passive_effect_types WHERE type = 'Spell' AND requirements = ?", {parts[0]});
			if (!effect.empty()) {
				trigger_effect(bot, event, p, "Spell", parts[0]);
			}
		}
		p.in_grimoire = false;
		claimed = true;
	} else if (custom_id == "cook" && !event.values.empty() && p.in_campfire && p.stamina > 0) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		if (parts.size() >= 1) {
			bot.log(dpp::ll_debug, event.command.usr.id.str() + ": attempting to cook: '" + parts[0] + "'");
			auto recipe = db::query(
				"SELECT food.id, food.name, food.description, GROUP_CONCAT(ingredient_name ORDER BY ingredient_name) AS ingredients, "
				"stamina_change, skill_change, luck_change, speed_change, value FROM food JOIN ingredients ON food_id = food.id "
				"WHERE food.name = ? GROUP BY food.id, food.name, food.description",
				{parts[0]}
			);
			if (recipe.empty()) {
				/* Recipe doesnt even exist, wtf */
				bot.log(dpp::ll_debug, event.command.usr.id.str() + ": invalid recipe name: '" + parts[0] + "'");
				return;
			}
			auto ingredients = db::query(
				"SELECT DISTINCT game_owned_items.id, item_desc FROM game_owned_items JOIN ingredients ON item_desc = ingredient_name WHERE user_id = ?"
				" UNION "
				"SELECT DISTINCT game_owned_items.id, item_desc FROM game_owned_items JOIN food ON item_desc = food.name WHERE user_id = ?",
				{event.command.usr.id, event.command.usr.id}
			);
			std::vector<std::string> recipe_ingredients = dpp::utility::tokenize(recipe[0].at("ingredients"), ",");
			std::vector<std::string> my_ingredients;
			for (auto& ingredient : ingredients) {
				my_ingredients.emplace_back(ingredient.at("item_desc"));
			}
			size_t checked{0};
			for (auto& check_ingredient : recipe_ingredients) {
				auto i = std::find(my_ingredients.begin(), my_ingredients.end(), check_ingredient);
				if (i != my_ingredients.end()) {
					my_ingredients.erase(i);
					p.drop_possession(item{ .name = check_ingredient, .flags = ""});
					checked++;
				}
			}
			if (checked < recipe_ingredients.size()) {
				/* Double-checked, can't cook this! */
				bot.log(dpp::ll_debug, event.command.usr.id.str() + ": player lacks ingredients for '" + parts[0] + "'; lag, bug, or hack attempt");
				return;
			}

			/* Replace ingredients with cooked meal */
			p.possessions.emplace_back(stacked_item{ .name = parts[0], .flags = "", .qty = 1});
			p.inv_change = true;
		}
		claimed = true;
	} else if (custom_id == "use_item" && !event.values.empty() && p.in_inventory && p.stamina > 0) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		if (parts.size() >= 2 && p.has_possession(parts[0])) {
			p.drop_possession(item{.name = parts[0], .flags = parts[1]});
			auto effect = db::query("SELECT * FROM passive_effect_types WHERE type = 'Consumable' AND requirements = ?", {parts[0]});
			if (!effect.empty()) {
				trigger_effect(bot, event, p, "Consumable", parts[0]);
			}
			auto food = db::query("SELECT * FROM food WHERE name = ?", {parts[0]});
			if (!food.empty()) {
				p.add_stamina(atol(food[0].at("stamina_change")));
				p.add_skill(atol(food[0].at("skill_change")));
				p.add_luck(atol(food[0].at("luck_change")));
				p.add_speed(atol(food[0].at("speed_change")));
			} else {
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
				p.armour.name = tr("NO_ARMOUR", event) + " 👙";
				p.armour.rating = 0;
			} else if (p.weapon.name == parts[0]) {
				p.weapon.name = tr("NO_WEAPON", event) + " 👊";
				p.weapon.rating = 0;
			}
			p.drop_possession(item{.name = parts[0], .flags = ""});
			p.add_gold(s.value);
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
	dpp::cluster& bot = *(event.from->creator);
	player p = get_live_player(event);
	bool claimed = false;
	if (p.state != state_play || event.custom_id.empty()) {
		return;
	}
	std::string custom_id = security::decrypt(event.custom_id);
	if (custom_id.empty()) {
		return;
	}
	event.from->log(dpp::ll_debug, event.command.locale + " " + std::to_string(event.command.usr.id) + ": " + custom_id);
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
		} else {
			bot.log(dpp::ll_warning, event.command.locale + " " + std::to_string(event.command.usr.id) + ": " + custom_id + " INVALID NAV FROM " + std::to_string(p.paragraph) + " TO " + std::to_string(dest));
		}
		claimed = true;
	} else if (parts[0] == "shop" && parts.size() >= 6) {
		std::string flags = parts[3];
		long cost = atol(parts[4]);
		std::string name = parts[5];
		if (p.paragraph != atol(parts[1])) {
			bot.log(dpp::ll_warning, event.command.locale + " " + std::to_string(event.command.usr.id) + ": " + custom_id + " INVALID SHOP FROM " + std::to_string(p.paragraph));
			return;
		}
		size_t max = p.max_inventory_slots();
		if (
			p.possessions.size() < max - 1 ||
			(
				dpp::lowercase(name) == "horse" || dpp::lowercase(name) == "pack pony" ||
				dpp::lowercase(name) == "donkey" || dpp::lowercase(name) == "mule" ||
				dpp::lowercase(name) == "pack" || dpp::lowercase(name) == "saddle bags" ||
				dpp::lowercase(name) == "backpack" || flags.substr(0, 4) == "CURE" ||
				flags == "SPELL" || flags == "HERB" || dpp::lowercase(name) == "scroll"
			)
		) {
			if (p.gold >= cost) {
				if (flags == "SPELL") {
					p.gold -= cost;
					name = replace_string(name, " ", "");
					name = replace_string(name, "-", "");
					name = replace_string(name, ".", "");
					if (!p.has_spell(name)) {
						p.spells.push_back(item{.name = dpp::lowercase(name), .flags = flags});
					}
				} else if (flags.substr(0, 4) == "CURE") {
					p.gold -= cost;
					try {
						if (flags == "CUREALL") {
							db::query("DELETE FROM kv_store WHERE user_id = ? AND (kv_key LIKE 'gamestate_lungrasp%' OR kv_key LIKE 'gamestate_blood_plague%' OR kv_key LIKE 'gamestate_bubonic_plague%' OR kv_key LIKE 'gamestate_green_rot%')", {event.command.usr.id});
							p.add_toast({ .message = tr("CURE_ALL", event), .image = "cure.png" });
						} else if (flags == "CURERASP") {
							db::query("DELETE FROM kv_store WHERE user_id = ? AND kv_key LIKE 'gamestate_lungrasp%'", {event.command.usr.id});
							p.add_toast({ .message = tr("CURE_RASP", event), .image = "cure.png" });
						} else if (flags == "CUREROT") {
							db::query("DELETE FROM kv_store WHERE user_id = ? AND kv_key LIKE 'gamestate_green_rot%'", {event.command.usr.id});
							p.add_toast({ .message = tr("CURE_ROT", event), .image = "cure.png" });
						} else if (flags == "CUREBLOOD") {
							db::query("DELETE FROM kv_store WHERE user_id = ? AND kv_key LIKE 'gamestate_blood_plague%'", {event.command.usr.id});
							p.add_toast({ .message = tr("CURE_BLOOD", event), .image = "cure.png" });
						} else if (flags == "CUREPLAGUE") {
							db::query("DELETE FROM kv_store WHERE user_id = ? AND kv_key LIKE 'gamestate_bubonic_plague%'", {event.command.usr.id});
							p.add_toast({ .message = tr("CURE_PLAGUE", event), .image = "cure.png" });
						}
					}
					catch (const regex_exception& e) {
						bot.log(dpp::ll_error, std::string("Regular expression error: ") + e.what());
					}
				} else if (flags == "HERB") {
					if (!p.has_herb(name)) {
						p.gold -= cost;
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
						stacked_item i{ .name = name, .flags = flags, .qty = 1 };
						if (!p.convert_rations(item{ .name = name, .flags = flags })) {
							bool special{false};
							if (dpp::lowercase(name) == "horse" || dpp::lowercase(name) == "pack pony" || dpp::lowercase(name) == "donkey" || dpp::lowercase(name) == "mule") {
								if (!p.has_flag("horse")) {
									p.gold -= cost;
									p.add_flag("horse");
									special = true;
								}
							} else if (dpp::lowercase(name) == "backpack" || dpp::lowercase(name) == "pack") {
								if (!p.has_flag("pack")) {
									p.gold -= cost;
									p.add_flag("pack");
									special = true;
								}
							} else if (dpp::lowercase(name) == "saddle bags") {
								if (!p.has_flag("saddlebags")) {
									p.gold -= cost;
									p.add_flag("saddlebags");
									special = true;
								}
							}
							if (!special) {
								p.pickup_possession(i);
								p.gold -= cost;
							}
						} else {
							p.gold -= cost;
						}
					}
				}
				p.inv_change = true;
			}
		}
		claimed = true;
	} else if (parts[0] == "combat" && parts.size() >= 7) {
		if (p.paragraph != atol(parts[1])) {
			bot.log(dpp::ll_warning, event.command.locale + " " + std::to_string(event.command.usr.id) + ": " + custom_id + " INVALID COMBAT FROM " + std::to_string(p.paragraph) + " TO " + parts[1]);
			return;
		}
		std::string monster_name{parts[2]};
		if (p.event.command.locale != "en") {
			auto translation = db::query("SELECT * FROM translations WHERE row_id = ? AND language = ? AND table_col = ?", {
				0, p.event.command.locale.substr(0, 2), monster_name
			});
			if (!translation.empty()) {
				monster_name = translation[0].at("translation");
			}
		}
		// paragraph name stamina skill armour weapon
		p.in_combat = true;
		p.combatant = enemy{
			.name = monster_name,
			.stamina = atol(parts[3]),
			.skill = atol(parts[4]),
			.armour = atol(parts[6]),
			.weapon = atol(parts[5]),
		};
		claimed = true;
	} else if (parts[0] == "bank" && !p.in_combat && !p.in_inventory) {
		if (p.paragraph != atol(parts[1])) {
			bot.log(dpp::ll_warning, event.command.locale + " " + std::to_string(event.command.usr.id) + ": " + custom_id + " INVALID BANK FROM " + std::to_string(p.paragraph) + " TO " + parts[1]);
			return;
		}
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
		if (p.paragraph != atol(parts[1])) {
			bot.log(dpp::ll_warning, event.command.locale + " " + std::to_string(event.command.usr.id) + ": " + custom_id + " INVALID PICK_ONE FROM " + std::to_string(p.paragraph) + " TO " + parts[1]);
			return;
		}
		p.paragraph = atol(parts[1]);
		size_t max = p.max_inventory_slots();
		if (p.possessions.size() < max - 1) {
			if (!p.has_flag("PICKED", p.paragraph)) {
				stacked_item i{.name = parts[3], .flags = parts[4], .qty = 1};
				if (!p.convert_rations(item{ .name = i.name, .flags = i.flags })) {
					p.pickup_possession(i);
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
		new_p.breadcrumb_trail = {};
		new_p.state = state_play;
		new_p.gold = p.gold;
		new_p.silver = p.silver;
		new_p.notoriety = p.notoriety;
		new_p.last_resurrect = p.last_resurrect;
		new_p.inv_change = true;
		new_p.toasts = {};
		new_p.death_xp_loss();
		new_p.reset_to_spawn_point();
		db::query("DELETE FROM timed_flags WHERE user_id = ?", { event.command.usr.id });
		db::query("DELETE FROM kv_store WHERE user_id = ?", { event.command.usr.id });
		update_live_player(event, new_p);
		new_p.save(event.command.usr.id);
		p = new_p;
		claimed = true;
	} else if (parts[0] == "resurrect") {
		time_t when = RESURRECT_SECS;
		auto rs = db::query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { event.command.usr.id });
		if (!event.command.entitlements.empty() || !rs.empty()) {
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
			p.toasts = {};
			p.state = state_play;
			update_live_player(event, p);
			p.save(event.command.usr.id);
			claimed = true;
		}
	} else if (parts[0] == "inventory" && parts.size() >= 2 && !p.in_combat && p.stamina > 0) {
		p.in_inventory = true;
		p.inventory_page = atoi(parts[1].c_str());
		claimed = true;
	} else if (parts[0] == "grimoire" && parts.size() >= 1 && !p.in_combat && p.stamina > 0) {
		p.in_grimoire = true;
		claimed = true;
	} else if (parts[0] == "campfire" && parts.size() >= 1 && !p.in_combat && p.stamina > 0) {
		p.in_campfire = true;
		claimed = true;
	} else if (parts[0] == "hunt" && parts.size() >= 2 && !p.in_combat && p.stamina > 0) {
		if (p.paragraph != atol(parts[1])) {
			bot.log(dpp::ll_warning, event.command.locale + " " + std::to_string(event.command.usr.id) + ": " + custom_id + " INVALID HUNT FROM " + std::to_string(p.paragraph) + " TO " + parts[1]);
			return;
		}
		if (p.possessions.size() >= p.max_inventory_slots()) {
			return;
		}
		auto rs = db::query("SELECT * FROM game_locations WHERE id = ?", {parts[1]});
		if (rs.empty()) {
			return;
		}
		std::stringstream ss;
		try {
			json hunt_data = json::parse(rs[0].at("hunting_json"));
			double probability = 100.0 - (hunt_data["probability"].get<double>() * 100);
			/* The probability of finding an animal at all is random between 0 and 100, but
			 * this random value is weighted against the player's profession. Woodsman profession
			 * have this multiplied by 0.75, but all other professions get it multiplied by 0.4,
			 * meaning if your job is not to hunt in the wilderness, you're going to have a harder
			 * time of it.
			 */
			double find_chance = (double)d_random(0, 100) * (double)(p.profession == prof_woodsman ? 0.75 : 0.4);
			auto bias = db::query("SELECT COUNT(*) AS current_ingredient_items FROM game_owned_items WHERE ((SELECT COUNT(*) FROM ingredients WHERE ingredient_name = item_desc LIMIT 1) > 0 OR (SELECT COUNT(*) FROM food WHERE food.name = item_desc LIMIT 1) > 0) AND user_id = ?", {event.command.usr.id});
			uint64_t current_ingredient_items = atol(bias[0].at("current_ingredient_items")), bias_factor{0};
			/* As you carry more and more ingredient items your probability of finding game animals decreases.
			 * This is a bias factor to prevent the user continually farming game animals all day long. There
			 * comes a limit where the odds of success are too great, and you have to sell, or take your
			 * cooked items to the bank and stash them, or consume them to carry on, encouraging a break in
			 * the game loop to do something else.
			 */
			if (current_ingredient_items >= 10 && current_ingredient_items < 15) {
				find_chance *= 0.5;
				bias_factor = 1;
			} else if (current_ingredient_items >= 15 && current_ingredient_items < 20) {
				find_chance *= 0.25;
				bias_factor = 2;
			} else if (current_ingredient_items >= 20 && current_ingredient_items < 25) {
				find_chance *= 0.125;
				bias_factor = 3;
			} else if (current_ingredient_items >= 25) {
				find_chance *= 0.05;
				bias_factor = 4;
			}
			ss << "## " << tr("HUNT_ATTEMPT", event) << "\n\n" << "*" << hunt_data["reason"].get<std::string>() << "*\n\n";
			std::vector<std::pair<std::string, json>> animals;
			for (auto &el: hunt_data["animals"].items()) {
				animals.emplace_back(el.key(), el.value());
			}
			/* Reverse so the rarest animal is at the start and most common at the end */
			reverse(begin(animals), end(animals));
			size_t animal_count = animals.size();
			bot.log(dpp::ll_debug, "Player hunting, probability of success=" + std::to_string(probability) + " score=" + std::to_string(find_chance) + " animal count=" + std::to_string(animal_count) + " bias factor=" + std::to_string(bias_factor));
			if (find_chance >= probability && animal_count > 0) {
				/* Hunted and found something */
				json animal = animals[0];
				/* When there are multiple animals possible to be hunted at the location,
				 * each successive animal is twice as unlikely to be encountered as the one
				 * before it. So, if there are two animals, the first animal has a probabity
				 * of 66% and the second has a probability of 33% of being encountered.
				 * With Three animals, this factors up again and with each, in increases by
				 * a square value. So, with the maximum of eight animals to pick from, the
				 * odds of encountering the eighth animal in the list are 128/1 while the odds
				 * of encountering the first are 50/50 (sucks to be the player who really
				 * wants that one animals parts!).
				 */
				std::array<int, 8> thresholds{1, 2, 4, 8, 16, 32, 64, 128};
				int x = d_random(1, 1 << (animal_count - 1));
				for (int i = 7; i >= 0; --i) {
					if (x >= thresholds[i]) {
						animal = animals[i].second;
						std::string animal_name{animals[i].first};
						if (event.command.locale.substr(0, 2) != "en") {
							auto t = db::query("SELECT * FROM translations WHERE row_id = 0 AND table_col = ? AND language = ?", {animal_name, event.command.locale.substr(0, 2)});
							if (!t.empty()) {
								animal_name = t[0].at("translation");
							}
						}
						ss << tr("SUCCESS_HUNT", event, animal_name) << "\n\n";
						break;
					}
				}
				/* Choice of animal part you get is completely random. The rarest animals are
				 * already 128/1 (0.7%) so having the same kind of thing AGAIN on the body
				 * parts would make some body parts ridiculously impossible to obtain.
				 */
				uint64_t random_animal_part = d_random(0, animal.size() - 1);
				std::string part = animal[random_animal_part].get<std::string>();
				auto i = tr(item{ .name = part, .flags = "" }, std::string{}, event);
				ss << "* 1x __" << i.name << "__\n";
				p.possessions.emplace_back(stacked_item{ .name = part, .flags = "", .qty = 1 });
				if (d12() == d12() && animal.size() > 1) {
					/* 1/12 chance of getting a second animal part, if the animal has more than one part */
					random_animal_part = d_random(0, animal.size() - 1);
					part = animal[random_animal_part].get<std::string>();
					i = tr(item{ .name = part, .flags = "" }, std::string{}, event);
					ss << "* 1x __" << i.name << "__\n";
					p.possessions.emplace_back(stacked_item{ .name = part, .flags = "", .qty = 1 });
				}
				p.inv_change = true;
			} else {
				ss << tr("FAILED_HUNT", event) << "\n";
				p.add_stamina(-1);
			}
			p.add_toast(toast{.message = ss.str(), .image = "hunting.png"});
		}
		catch (const std::exception& e) {
			/* We end up here if the hunting_json is invalid or empty */
			ss << "## " << tr("HUNT_ATTEMPT", event) << "\n\n" << "*" << tr("NOTHING_HUNT", event) << "*\n\n" << tr("FAILED_HUNT", event) << "\n";
			p.add_toast(toast{.message = ss.str(), .image = "hunting.png"});
			p.add_stamina(-1);
			if (!rs[0].at("hunting_json").empty()) {
				bot.log(dpp::ll_error, "Error in hunting, location " + std::to_string(p.paragraph) + ": " + std::string(e.what()));
			}
		}
		claimed = true;
	} else if (parts[0] == "pick" && parts.size() >= 4 && !p.in_inventory && p.stamina > 0) {
		/* Pick up frm floor */
		if (p.paragraph != atol(parts[1])) {
			bot.log(dpp::ll_warning, event.command.locale + " " + std::to_string(event.command.usr.id) + ": " + custom_id + " INVALID PICKUP FROM " + std::to_string(p.paragraph) + " TO " + parts[1]);
			return;
		}
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
				stacked_item i{.name = name, .flags = flags, .qty = 1 };
				if (!p.convert_rations({.name = name, .flags = flags})) {
					p.pickup_possession(i);
				}
				p.inv_change = true;
				send_chat(event.command.usr.id, p.paragraph, name, "pickup");
			}
			db::commit();
		}
		claimed = true;
	} else if (parts[0] == "exit_inventory" && parts.size() == 1 && !p.in_combat) {
		p.in_inventory = false;
		claimed = true;
	} else if (parts[0] == "exit_grimoire" && parts.size() == 1 && !p.in_combat) {
		p.in_grimoire = false;
		claimed = true;
	} else if (parts[0] == "exit_campfire" && parts.size() == 1 && !p.in_combat) {
		p.in_campfire = false;
		claimed = true;
	} else if (parts[0] == "exit_bank" && parts.size() == 1 && !p.in_combat) {
		p.in_bank = false;
		claimed = true;
	} else if (parts[0] == "refresh") {
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
		dpp::message m = dpp::message(tr("PVP_REJECTED", event, "<@" + opponent.str() +  ">",  p.name)).set_allowed_mentions(true, false, false, false, {}, {});
		m.channel_id = p2.event.command.channel_id;
		m.guild_id = p2.event.command.guild_id;
		m.add_component(
			dpp::component()
			.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("exit_pvp_picker"))
				.set_label(tr("GO_BACK", event))
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
			.set_label(tr("ENTER_MESSAGE", event))
			.set_id(security::encrypt("chat_message"))
			.set_type(dpp::cot_text)
			.set_placeholder(tr("HELLO", event))
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
	auto food = db::query("SELECT * FROM food WHERE name = ?", {name});
	if (!food.empty()) {
		return sprite::cheese;
	}
	auto ingredient = db::query("SELECT * FROM ingredients WHERE ingredient_name = ?", {name});
	if (!ingredient.empty()) {
		return sprite::rawmeat;
	}
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
	content << "__**" << tr("BANK_WELCOME", event) << "**__\n\n";
	content << tr("BANK_MAX", event, p.max_gold()) << "\n";
	content << tr("SILVER_UPSELL", event) << "\n";

	std::ranges::sort(p.possessions, [](const stacked_item &a, const stacked_item& b) -> bool { return a.name < b.name; });
	auto bank_items = db::query("SELECT owner_id, item_desc, item_flags, COUNT(item_desc) AS qty FROM `game_bank` where owner_id = ? and item_desc != ? GROUP BY owner_id, item_desc, item_flags ORDER BY item_desc LIMIT 25",{event.command.usr.id, "__GOLD__"});
	if (!bank_items.empty()) {
		content << "\n__**" << bank_items.size() << "/25 " << tr("BANK_ITEMS", event) << "**__\n";
		for (const auto& bank_item : bank_items) {
			auto i = tr(item{ .name = bank_item.at("item_desc"), .flags = bank_item.at("item_flags") }, "", event);
			long item_qty = atol(bank_item.at("qty"));
			content << sprite::backpack.get_mention() << " " << i.name;
			if (item_qty > 1) {
				content << " (x" << item_qty << ")";
			}
			content << "\n";
		}
	}

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = tr("BANK", event),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.add_field(tr("YOUR_BALANCE", event), std::to_string(amount) + " " + tr("GOLD", event) + " " + sprite::gold_coin.get_mention(), true)
		.add_field(tr("COIN_PURSE", event), std::to_string(p.gold) + " " + tr("GOLD", event) + " " + sprite::gold_coin.get_mention(), true)
		.set_colour(EMBED_COLOUR)
		.set_description(content.str());
	
	dpp::message m;
	dpp::component deposit_menu, withdraw_menu;

	deposit_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder(tr("DEPOSIT_ITEM", event))
		.set_id(security::encrypt("deposit"));
	size_t index{0};
	std::set<std::string> ds;
	for (const auto& inv : p.possessions) {
		sale_info si = get_sale_info(inv.name);
		auto i = tr(inv, "", event);
		if (si.quest_item || dpp::lowercase(inv.name) == "scroll") {
			/* Can't bank a scroll! */
			continue;
		}
		if (ds.find(inv.name) == ds.end()) {
			dpp::emoji e = get_emoji(inv.name, inv.flags);
			if (deposit_menu.options.size() < 25) {
				deposit_menu.add_select_option(
					dpp::select_option(i.name, inv.name + ";" + inv.flags)
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
		.set_placeholder(tr("WITHDRAW_ITEM", event))
		.set_id(security::encrypt("withdraw"));
	std::set<std::string> dup_set;
	for (const auto& bank_item : bank_items) {
		if (dup_set.find(bank_item.at("item_desc")) == dup_set.end()) {
			dpp::emoji e = get_emoji(bank_item.at("item_desc"), bank_item.at("item_flags"));
			auto i = tr(item{ .name = bank_item.at("item_desc"), .flags = bank_item.at("item_flags") }, "", event);
			if (withdraw_menu.options.size() < 25) {
				withdraw_menu.add_select_option(
					dpp::select_option(i.name, bank_item.at("item_desc") + ";" + bank_item.at("item_flags"))
					.set_emoji(e.name, e.id)
				);
				dup_set.insert(bank_item.at("item_desc"));
			}
		}
	}

	m.add_embed(embed);
	if (dup_set.size() < 25 && index > 0 && withdraw_menu.options.size() < 25) {
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
			.set_label(tr("BACK", event))
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::magic05.name, sprite::magic05.id)
		)
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("deposit_gold"))
			.set_label(tr("DEPOSIT_GOLD", event))
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::gold_coin.name, sprite::gold_coin.id)
			.set_disabled(p.gold == 0)
		)
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("withdraw_gold"))
			.set_label(tr("WITHDRAW_GOLD", event))
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::gold_coin.name, sprite::gold_coin.id)
			.set_disabled(amount == 0)
		)
		.add_component(help_button(event))
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
		content << tr("PVP_SELECT", event);
	} else {
		content << tr("PVP_WAIT", event, sprite::magic05.get_mention());
	}

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = tr("PVP_SEL_FOOTER", event, p.name),
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
			.set_placeholder(tr("SELECT_PLAYER", event))
			.set_id(security::encrypt("fight_pvp"));
		for (const auto& other: others) {
			fight_menu.add_select_option(
				dpp::select_option(other.at("name"), other.at("user_id"), tr("STAMINA", event) + ": " + other.at("stamina") + ", " + tr("SKILL", event) + ": " + other.at("skill") + ", " + other.at("experience") + " XP")
			);
		}
	} else {
		content << "\n" << tr("CRICKETS", event);
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
			.set_label(tr("BACK", event))
			.set_style(dpp::cos_secondary)
			.set_emoji(sprite::magic05.name, sprite::magic05.id)
		)
		.add_component(help_button(event))
	);

	event.reply(event.command.type == dpp::it_application_command || event.command.type == dpp::it_component_button ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, cc.http_info.body);
			event.reply(dpp::message(tr("GONE_AWAY", event)).add_component(
				dpp::component()
				.add_component(dpp::component()
				       .set_type(dpp::cot_button)
				       .set_id(security::encrypt("exit_pvp_picker"))
				       .set_label(tr("BACK", event))
				       .set_style(dpp::cos_secondary)
				       .set_emoji(sprite::magic05.name, sprite::magic05.id)
				)
				.add_component(help_button(event))
			)
			.add_component(help_button(event)).set_flags(dpp::m_ephemeral));
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
	} else if (p.in_grimoire) {
		grimoire(event, p);
		return;
	} else if (p.in_campfire) {
		campfire(event, p);
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
			.text = tr("LOCATION", event) + " " + location.secure_id,
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
			text += "**__" + tr("OTHERS", event) + "__**\n" + list_others + "\n\n";
		}
		if (location.dropped_items.size()) {
			for (const auto & dropped : location.dropped_items) {
				item dropped_single{ .name = dropped.name, .flags = dropped.flags };
				auto i = tr(dropped_single, "", event);
				list_dropped += dpp::utility::markdown_escape(i.name, true);
				if (dropped.qty > 1) {
					list_dropped += " (x " + std::to_string(dropped.qty) + ")";
				}
				list_dropped += ", ";
			}
			if (list_dropped.length()) {
				list_dropped = list_dropped.substr(0, list_dropped.length() - 2);
				text += "**__" + tr("ITEMS", event) + "__**\n" + list_dropped + "\n\n";
			}
		}
		add_chat(text, p.event, location.id, get_guild_id(p));
		m.add_embed(dpp::embed()
			.set_colour(EMBED_COLOUR)
			.set_description(text)
		);
	} else {
		std::string text = "";
		add_chat(text, p.event, location.id, get_guild_id(p));
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
			.set_placeholder(tr("SELL_ITEM", event))
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
					auto i = tr(inv, "", event);
					sell_menu.add_select_option(
						dpp::select_option(i.name, inv.name + ";" + inv.flags, tr("VALUE", event) + " " + std::to_string(s.value) + " - " + describe_item(inv.flags, inv.name, event).substr(0, 80))
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
				label = tr("PAYLINK", event, n.cost);
				id = "follow_nav_pay;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::to_string(n.cost) + ";" + std::to_string(++unique);
				enabled_links++;
				break;
			case nav_type_pick_one:
				// PICKED
				label = tr("CHOOSE", event, n.buyable.name);
				id = "pick_one;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + n.buyable.name + ";" + n.buyable.flags + ";" + std::to_string(++unique);
				enabled_links++;
				break;
			case nav_type_shop:
				label = tr("BUY", event, n.buyable.name, n.cost);
				id = "shop;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::string(n.buyable.flags) + ";" + std::to_string(n.cost) + ";" + n.buyable.name + ";" + std::to_string(++unique);
				if (p.has_herb(n.buyable.name) || p.has_spell(n.buyable.name) || p.gold < n.cost) {
					comp.set_disabled(true);
				} else {
					enabled_links++;
				}
				break;
			case nav_type_bank:
				label = tr("USE_BANK", event);
				id = "bank;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::to_string(++unique);
				enabled_links++;
				break;
			case nav_type_combat:
				label = tr("FIGHT", event, n.monster.name);
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
				label = tr("ANSWER", event, n.prompt);
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
	} else if (p.stamina < 1) {
		death(p, cb);
	}

	do_toasts(p, cb);

	p.save(event.command.usr.id);
	update_live_player(event, p);

	if (enabled_links > 0 && p.stamina > 0) {
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("inventory;0"))
			.set_label(tr("INVENTORY", event))
			.set_style(dpp::cos_secondary)
			.set_emoji(sprite::backpack.name, sprite::backpack.id)
		);

		cb.add_component(dpp::component()
			 .set_type(dpp::cot_button)
			 .set_id(security::encrypt("grimoire"))
			 .set_label(tr("SPELLS", event))
			 .set_style(dpp::cos_secondary)
			 .set_emoji(sprite::book07.name, sprite::book07.id)
		);

		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("hunt;" +std::to_string(p.paragraph)))
			.set_label(tr("HUNT", event))
			.set_style(dpp::cos_secondary)
			.set_emoji("🦌")
			.set_disabled(p.possessions.size() >= p.max_inventory_slots())
		);

		auto r = db::query("SELECT hunting_json FROM game_locations WHERE id = ?", {p.paragraph});
		if (!r.empty() && !r[0].at("hunting_json").empty()) {
			try {
				json hunt_data = json::parse(r[0].at("hunting_json"));
				if (hunt_data["probability"].get<double>() > 0) {
					cb.add_component(dpp::component()
						.set_type(dpp::cot_button)
						.set_id(security::encrypt("campfire"))
						.set_label(tr("COOK", event))
						.set_style(dpp::cos_secondary)
						.set_emoji(sprite::cooked_meat.name, sprite::cooked_meat.id)
					);
				}
			}
			catch (...) {}
		}
	}

	if (enabled_links > 0 && p.stamina > 0 && p.after_fragment == 0) {
		if (!others.empty()) {
			/* Can fight other players, present option */
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("pvp_picker"))
				.set_label(tr("PVP", event))
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
			item dropped_single{ .name = dropped.name, .flags = dropped.flags };
			auto i = tr(dropped_single, "", event);
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("pick;" + std::to_string(p.paragraph) + ";" + dropped.name + ";" + dropped.flags))
				.set_label(tr("PICK", event, dropped.name))
				.set_style(dpp::cos_secondary)
				.set_emoji(sprite::backpack.name, sprite::backpack.id)
			);
		}
	}

	cb.add_component(help_button(event));
	m = cb.get_message();

	event.reply(event.command.type == dpp::it_component_button ? dpp::ir_update_message : dpp::ir_channel_message_with_source, m.set_flags(dpp::m_ephemeral), [event, &bot, location, m](const auto& cc) {
		if (cc.is_error()) {{
			bot.log(dpp::ll_error, "Internal error displaying location " + std::to_string(location.id) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
			event.reply(dpp::message("Internal error displaying location " + std::to_string(location.id) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```").set_flags(dpp::m_ephemeral));
		}}
	});
}
