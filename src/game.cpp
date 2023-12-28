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
#include <ssod/emojis.h>
#include <ssod/combat.h>

void game_input(const dpp::form_submit_t & event) {
	if (!player_is_live(event)) {
		return;
	}
	player p = get_live_player(event);
	dpp::cluster& bot = *(event.from->creator);
	bot.log(dpp::ll_debug, std::to_string(event.command.usr.id) + ": " + event.custom_id);
	bool claimed{true};
	if (event.custom_id == "deposit_gold_amount_modal" && p.in_bank) {
		long amount = std::max(0l, atol(std::get<std::string>(event.components[0].components[0].value)));
		amount = std::min(amount, p.gold);
		if (p.gold > 0 && amount > 0) {
			p.add_gold(-amount);
			db::query("INSERT INTO game_bank (owner_id, item_desc, item_flags) VALUES(?,'__GOLD__',?)", {event.command.usr.id, amount});
		}
		claimed = true;
	} else if (event.custom_id == "withdraw_gold_amount_modal" && p.in_bank) {
		long amount = std::max(0l, atol(std::get<std::string>(event.components[0].components[0].value)));
		auto bank_amount = db::query("SELECT SUM(item_flags) AS gold FROM game_bank WHERE owner_id = ? AND item_desc = ?",{event.command.usr.id, "__GOLD__"});
		long balance_amount = atol(bank_amount[0].at("gold"));
		amount = std::min(amount, balance_amount);
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
	bot.log(dpp::ll_debug, std::to_string(event.command.usr.id) + ": " + event.custom_id);
	if (event.custom_id == "withdraw" && p.in_bank) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		db::query("DELETE FROM game_bank WHERE owner_id = ? AND item_desc = ? AND item_flags = ?", {event.command.usr.id, parts[0], parts[1]});
		p.possessions.push_back(item{ .name = parts[0], .flags = parts[1] });
		claimed = true;
	} else if (event.custom_id == "deposit" && p.in_bank) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.values[0], ";");
		db::query("INSERT INTO game_bank (owner_id, item_desc, item_flags ) VALUES(?,?,?)", {event.command.usr.id, parts[0], parts[1]});
		p.drop_possession(item{ .name = parts[0], .flags = parts[1] });
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
	event.from->log(dpp::ll_debug, std::to_string(event.command.usr.id) + ": " + event.custom_id);
	std::vector<std::string> parts = dpp::utility::tokenize(event.custom_id, ";");
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
			p.after_fragment = 0; // Resets current combat index
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
			.armour = atol(parts[6]),
			.weapon = atol(parts[5]),
		};
		claimed = true;
	} else if (parts[0] == "bank" && !p.in_combat && !p.in_inventory) {
		p.in_bank = true;
		claimed = true;
	} else if (parts[0] == "deposit_gold" && p.in_bank) {
		dpp::interaction_modal_response modal("deposit_gold_amount_modal", "Deposit Gold",	{
			dpp::component()
			.set_label("Enter Gold Amount")
			.set_id("deposit_gold_amount")
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
		dpp::interaction_modal_response modal("withdraw_gold_amount_modal", "Withdraw Gold",	{
			dpp::component()
			.set_label("Enter Gold Amount")
			.set_id("withdraw_gold_amount")
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
		if (!p.has_flag("PICKED", p.paragraph)) {
			p.possessions.push_back(item{ .name = parts[3], .flags = parts[4] });
			p.add_flag("PICKED", p.paragraph);
		}
		claimed = true;
	} else if (parts[0] == "respawn") {
		/* Load backup of player and save over the current */
		player new_p = player(event.command.usr.id, true);
		/* Keep experience points only (HARDCORE!!!) */
		new_p.experience = p.experience;
		new_p.in_combat = false;
		new_p.after_fragment = 0;
		new_p.combatant = {};
		new_p.possessions = {};
		new_p.state = state_play;
		new_p.gold = p.gold;
		new_p.silver = p.silver;
		new_p.reset_to_spawn_point();
		update_live_player(event, new_p);
		new_p.save(event.command.usr.id);
		p = new_p;
		claimed = true;
	} else if (parts[0] == "inventory" && parts.size() >= 1 && !p.in_combat && p.stamina > 0) {
		p.in_inventory = true;
		claimed = true;
	} else if (parts[0] == "drop" && parts.size() >= 3 && p.in_inventory && p.stamina > 0) {
		p.drop_possession(item{ .name = parts[1], .flags = parts[2] });
		if (p.armour.name == parts[1]) {
			p.armour.name = "Undergarments 👙";
			p.armour.rating = 0;
		} else if (p.weapon.name == parts[1]) {
			p.weapon.name = "Unarmed 👊";
			p.weapon.rating = 0;
		}
		/* Drop to floor */
		db::query("INSERT INTO game_dropped_items (location_id, item_desc, item_flags) VALUES(?,?,?)", {p.paragraph, parts[1], parts[2]});
		claimed = true;
	} else if (parts[0] == "pick" && parts.size() >= 4 && !p.in_inventory && p.stamina > 0) {
		/* Pick up frm floor */
		long paragraph = atol(parts[1]);
		std::string name = parts[2];
		std::string flags = parts[3];
		db::query("DELETE FROM game_dropped_items WHERE location_id = ? AND item_desc = ? AND item_flags = ? LIMIT 1", {paragraph, name, flags});
		p.possessions.push_back(item{ .name = name, .flags = flags });
		claimed = true;
	} else if (parts[0] == "use" && parts.size() >= 3 && p.in_inventory && p.stamina > 0) {
		p.drop_possession(item{ .name = parts[1], .flags = parts[2] });
		std::string flags = parts[2];
		if (flags.substr(0, 2) == "ST") {
			long modifier = atol(flags.substr(2, flags.length() - 2));
			p.add_stamina(modifier);
		} else if (flags.substr(0, 2) == "SK") {
			long modifier = atol(flags.substr(2, flags.length() - 2));
			p.add_skill(modifier);
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
		claimed = true;
	} else if (parts[0] == "equip" && parts.size() >= 3 && p.in_inventory && p.stamina > 0) {
		if (parts[1][0] == 'W') {
			p.weapon = rated_item{ .name = parts[1], .rating = atol(parts[2]) };
		} else {
			p.armour = rated_item{ .name = parts[1], .rating = atol(parts[2]) };
		}
		claimed = true;
	} else if (parts[0] == "exit_inventory" && parts.size() == 1 && !p.in_combat && p.stamina > 0) {
		p.in_inventory = false;
		claimed = true;
	} else if (parts[0] == "exit_bank" && parts.size() == 1 && !p.in_combat && p.stamina > 0) {
		p.in_bank = false;
		claimed = true;
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

void inventory(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	std::stringstream content;

	content << "__**Stats**__\n\n";
	content << "<:" << sprite::health_heart.format() << "> Stamina: __" << p.stamina << "__";
	content << " <:" << sprite::book07.format() << "> Skill: __" << p.skill << "__";
	content << " <:" << sprite::clover.format() << "> Luck: __" << p.luck << "__";
	content << " <:" << sprite::medal01.format() << "> XP: __" << p.experience << "__";
	content << " <:" << sprite::shoes03.format() << "> Speed: __" << p.speed << "__";
	content << "\n";

	content << "\n__**Inventory**__\n";
	if (p.gold > 0) {
		content << "<:" << sprite::gold_coin.format() << ">" << " " << p.gold << " Gold Pieces\n";
	}
	if (p.silver > 0) {
		content << "<:" << sprite::silver_coin.format() << ">" << " " << p.gold << " Silver Pieces\n";
	}
	std::ranges::sort(p.possessions, [](const item &a, const item& b) -> bool { return a.name < b.name; });
	for (const auto& inv : p.possessions) {
		std::string emoji = get_emoji(inv.name, inv.flags).format();
		content << "<:" << emoji << ">" << " " << inv.name << " - *" << describe_item(inv.flags, inv.name) << "*";
		if (p.armour.name == inv.name || p.weapon.name == inv.name) {
			content << " <:" << sprite::light02.format() << "> - **Equipped**";
		}
		content << "\n";
	}
	content << "\n__**Spells**__\n";
	std::ranges::sort(p.spells, [](const item &a, const item& b) -> bool { return a.name < b.name; });
	for (const auto& inv : p.spells) {
		content << "<:" << sprite::hat02.format() << ">" << " " << inv.name << "\n";
	}
	content << "\n__**Herbs**__\n";
	std::ranges::sort(p.herbs, [](const item &a, const item& b) -> bool { return a.name < b.name; });
	for (const auto& inv : p.herbs) {
		content << "<:" << sprite::leaf.format() << ">" << " " << inv.name << "\n";
	}

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = "Inventory",
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_description(content.str());
	
	dpp::message m;
	m.add_embed(embed);
	component_builder cb(m);

	cb.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id("exit_inventory")
		.set_label("Back")
		.set_style(dpp::cos_primary)
		.set_emoji(sprite::magic05.name, sprite::magic05.id)
	);

	size_t index{0};
	for (const auto& inv : p.possessions) {
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id("drop;" + inv.name + ";" + inv.flags + ";" + std::to_string(++index))
			.set_label("Drop " + inv.name)
			.set_style(dpp::cos_danger)
			.set_emoji(sprite::inv_drop.name, sprite::inv_drop.id)
		);
		if (inv.flags.find("+") != std::string::npos) {
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id("use;" + inv.name + ";" + inv.flags + ";" + std::to_string(++index))
				.set_label("Use " + inv.name)
				.set_style(dpp::cos_success)
				.set_emoji("➕")
			);
		} else if (inv.flags.length() && inv.flags[0] == 'A') {
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id("equip;" + inv.name + ";" + inv.flags + ";" + std::to_string(++index))
				.set_label("Wear " + inv.name)
				.set_style(dpp::cos_secondary)
				.set_emoji(sprite::armor04.name, sprite::armor04.id)
			);
		}
	}

	m = cb.get_message();

	event.reply(event.command.type == dpp::it_application_command ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying inventory:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
			event.reply("Internal error displaying inventory:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
		}
	});
}

void bank(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	std::stringstream content;

	auto bank_amount = db::query("SELECT SUM(item_flags) AS gold FROM game_bank WHERE owner_id = ? AND item_desc = ?",{event.command.usr.id, "__GOLD__"});
	long amount = atol(bank_amount[0].at("gold"));
	content << "__**Welcome to the Bank Of Utopia**__\n\n";
	content << "Your balance: " + std::to_string(amount) + " Gold " + sprite::gold_coin.get_mention() + "\n";
	content << "Coin purse: " + std::to_string(p.gold) + " Gold " + sprite::gold_coin.get_mention() + "\n";

	std::ranges::sort(p.possessions, [](const item &a, const item& b) -> bool { return a.name < b.name; });
	auto bank_items = db::query("SELECT * FROM game_bank WHERE owner_id = ? AND item_desc != ? ORDER BY item_desc LIMIT 25",{event.command.usr.id, "__GOLD__"});
	if (bank_items.size() > 0) {
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
		.set_colour(0xd5b994)
		.set_description(content.str());
	
	dpp::message m;
	dpp::component deposit_menu, withdraw_menu;

	deposit_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder("Deposit Item")
		.set_id("deposit");
	size_t index{0};
	for (const auto& inv : p.possessions) {
		if (dpp::lowercase(inv.name) == "scroll") {
			/* Can't bank a scroll! */
			continue;
		}
		dpp::emoji e = get_emoji(inv.name, inv.flags);
		deposit_menu.add_select_option(
			dpp::select_option(inv.name, inv.name + ";" + inv.flags, describe_item(inv.flags, inv.name).substr(0, 100))
			.set_emoji(e.name, e.id)
		);
		if (index++ == 25) {
			break;
		}
	}
	withdraw_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder("Withdraw Item")
		.set_id("withdraw");
	for (const auto& bank_item : bank_items) {
		dpp::emoji e = get_emoji(bank_item.at("item_desc"), bank_item.at("item_flags"));
		withdraw_menu.add_select_option(
			dpp::select_option(bank_item.at("item_desc"), bank_item.at("item_desc") + ";" + bank_item.at("item_flags"), describe_item(bank_item.at("item_flags"), bank_item.at("item_desc")).substr(0, 100))
			.set_emoji(e.name, e.id)
		);
	}

	if (bank_items.size() < 25) {
		m.add_embed(embed).add_component(dpp::component().add_component(deposit_menu));
	}
	if (bank_items.size() > 0) {
		m.add_component(dpp::component().add_component(withdraw_menu));
	}
	m.add_component(
		dpp::component()
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id("exit_bank")
			.set_label("Back")
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::magic05.name, sprite::magic05.id)
		)
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id("deposit_gold")
			.set_label("Deposit Gold")
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::gold_coin.name, sprite::gold_coin.id)
			.set_disabled(p.gold == 0)
		)
		.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id("withdraw_gold")
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
			event.reply("Internal error displaying bank:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
		}
	});
}

void continue_game(const dpp::interaction_create_t& event, player p) {
	if (p.in_combat) {
		continue_combat(event, p);
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
		.set_colour(0xd5b994)
		.set_description(location.text);
	p.save(event.command.usr.id);
	update_live_player(event, p);
	dpp::message m;
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
		m.add_embed(dpp::embed()
			.set_colour(0xd5b994)
			.set_description(text)
		);
	}

	component_builder cb(m);
	size_t index{0}, enabled_links{0};
	bool respawn_button_shown{false};
	size_t unique{0};

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
				label = "Respawn";
				id = "respawn";
				p.stamina = 0; /* 💀 */
				p.drop_everything();
				p.save(event.command.usr.id);
				update_live_player(event, p);
				break;
			default:
				id = "follow_nav;" + std::to_string(n.paragraph) + ";" + std::to_string(p.paragraph) + ";" + std::to_string(++unique);
				enabled_links++;
				break;
		}
		comp.set_type(dpp::cot_button)
			.set_id(id)
			.set_label(label)
			.set_style(n.type == nav_type_combat ? dpp::cos_danger : dpp::cos_primary)
			.set_emoji(directions[++index], 0, false);

		if (n.type == nav_type_respawn) {
			comp.set_emoji(sprite::skull.name, sprite::skull.id);
		} else if (n.type == nav_type_bank) {
			comp.set_emoji(sprite::gold_bar.name, sprite::gold_bar.id);
		}
		cb.add_component(comp);
	}
	if (enabled_links == 0 && !respawn_button_shown) {
		p.stamina = 0;
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id("respawn")
			.set_label("Respawn")
			.set_style(dpp::cos_danger)
			.set_emoji(sprite::skull.name, sprite::skull.id)
		);
		p.drop_everything();
		p.save(event.command.usr.id);
		update_live_player(event, p);
	}
	if (enabled_links > 0 && p.stamina > 0 && p.after_fragment == 0) {
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id("inventory")
			.set_label("Inventory")
			.set_style(dpp::cos_secondary)
			.set_emoji(sprite::backpack.name, sprite::backpack.id)
		);

		for (const auto & dropped : location.dropped_items) {
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id("pick;" + std::to_string(p.paragraph) + ";" + dropped.name + ";" + dropped.flags)
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
			event.reply("Internal error displaying location " + std::to_string(location.id) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
		}}
	});
}
