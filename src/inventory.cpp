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
#include <ssod/game_util.h>
#include <ssod/component_builder.h>
#include <ssod/emojis.h>
#include <ssod/aes.h>
#include <ssod/wildcard.h>

void inventory(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	std::stringstream content;
	bool equip_w{false}, equip_a{false};

	size_t pages_max = ceil(p.max_inventory_slots() / 25);
	if (p.inventory_page > pages_max) {
		p.inventory_page = 0;
	}

	std::vector<dpp::embed_field> fields;

	content << "__**Stats**__\n";
	content << "<:" << sprite::health_heart.format() << "> Stamina: __" << p.stamina << "__";
	content << " <:" << sprite::book07.format() << "> Skill: __" << p.skill << "__";
	content << " <:" << sprite::clover.format() << "> Luck: __" << p.luck << "__";
	content << " <:" << sprite::medal01.format() << "> XP: __" << p.experience << "__ (Level: __" << p.get_level() << "__)\n";
	content << " <:" << sprite::shoes03.format() << "> Speed: __" << p.speed << "__";
	if (p.gold > 0) {
		content << " <:" << sprite::gold_coin.format() << "> Gold: __" << p.gold << "__";
	}
	if (p.silver > 0) {
		content << " <:" << sprite::silver_coin.format() << "> Silver: __" << p.silver << "__";
	}

	content << "\n";
	if (p.has_flag("pack") || p.has_flag("horse") || p.has_flag("steamcopter") || p.has_flag("saddlebags")) {
		content << "\n__**Storage**__\n";
		if (p.has_flag("pack")) {
			content << sprite::backpack.get_mention() << " Backpack (__+1__) ";
		}
		if (p.has_flag("horse")) {
			content << "🐴 Horse (__+1__) ";
		}
		if (p.has_flag("saddlebags")) {
			content << sprite::backpack.get_mention() << " Saddle Bags (__+1__) ";
		}
		if (p.has_flag("steamcopter")) {
			content << "🚁 Steam-Copter (__+3__) ";
		}
		content << "\n";
	}

	content << "\n__**Spells**__\n";
	std::ranges::sort(p.spells, [](const item &a, const item &b) -> bool { return a.name < b.name; });
	for (const auto &inv: p.spells) {
		content << "<:" << sprite::hat02.format() << ">" << " " << human_readable_spell_name(inv.name) << "\n";
	}
	content << "\n__**Herbs**__\n";
	std::ranges::sort(p.herbs, [](const item &a, const item &b) -> bool { return a.name < b.name; });
	for (const auto &inv: p.herbs) {
		content << "<:" << sprite::leaf.format() << ">" << " " << human_readable_herb_name(inv.name) << "\n";
	}

	content << "\n__**Inventory (page " << (p.inventory_page + 1) << " of " << pages_max ++ << ")**__\n";
	std::vector<item> possessions = p.possessions_page(p.inventory_page);
	std::ranges::sort(possessions, [](const item &a, const item& b) -> bool { return a.name < b.name; });
	for (const auto& inv : possessions) {
		std::string emoji = get_emoji(inv.name, inv.flags).format();
		std::string description{"```ansi\n" + describe_item(inv.flags, inv.name, true) + "\n"};
		if (p.armour.name == inv.name && !equip_a) {
			description += "\033[2;31m🫱🏼 Equipped\033[0m ";
			equip_a = true;
		} else if (p.weapon.name == inv.name && !equip_w) {
			description += "\033[2;31m🫱🏼 Equipped\033[0m ";
			equip_w = true;
		}
		sale_info value = get_sale_info(inv.name);
		if (value.quest_item) {
			description += "\033[2;32m❗ Quest Item\033[0m ";
		}
		if (value.sellable && !value.quest_item) {
			description += "\033[2;33m🪙 " + std::to_string(value.value) + " Value\033[0m ";
		}
		description += "\n```\n";
		fields.emplace_back(dpp::embed_field("<:" + emoji + "> " + inv.name, description, true));
	}

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{
			.text = p.name + "'s Inventory (Page " + std::to_string(p.inventory_page + 1) + ")",
			.icon_url = bot.me.get_avatar_url(),
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(content.str());
	embed.fields = fields;

	dpp::message m;
	m.add_embed(embed);
	component_builder cb(m);
	size_t index{0};

	cb.add_component(dpp::component()
				 .set_type(dpp::cot_button)
				 .set_id(security::encrypt("exit_inventory"))
				 .set_label("Back")
				 .set_style(dpp::cos_primary)
				 .set_emoji(sprite::magic05.name, sprite::magic05.id)
	);

	cb.add_component(dpp::component()
				 .set_type(dpp::cot_button)
				 .set_id(security::encrypt("inventory;" + std::to_string(p.inventory_page - 1)))
				 .set_style(dpp::cos_secondary)
				 .set_emoji("◀\uFE0F")
				 .set_disabled(p.inventory_page <= 0)
	);
	cb.add_component(dpp::component()
				 .set_type(dpp::cot_button)
				 .set_id(security::encrypt("inventory;" + std::to_string(p.inventory_page + 1)))
				 .set_style(dpp::cos_secondary)
				 .set_emoji("▶")
				 .set_disabled(p.inventory_page >= pages_max - 2)
	);

	m = cb.get_message();

	dpp::component use_menu, drop_menu, equip_menu;

	use_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder("Use Item")
		.set_id(security::encrypt("use_item"));
	drop_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder("Drop Item")
		.set_id(security::encrypt("drop_item"));
	equip_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder("Equip Item")
		.set_id(security::encrypt("equip_item"));

	for (const auto& inv : possessions) {
		sale_info value = get_sale_info(inv.name);
		dpp::emoji e = get_emoji(inv.name, inv.flags);
		std::string desc = replace_string(describe_item(inv.flags, inv.name), "**", "").substr(0, 80);
		if (!value.quest_item && value.sellable) {
			drop_menu.add_select_option(dpp::select_option("Drop " + inv.name, inv.name + ";" + inv.flags + ";" + std::to_string(++index), desc).set_emoji(sprite::inv_drop.name, sprite::inv_drop.id));
		}
		if (inv.flags.find('+') != std::string::npos || inv.flags.find('-') != std::string::npos) {
			use_menu.add_select_option(dpp::select_option("Use " + inv.name, inv.name + ";" + inv.flags + ";" + std::to_string(++index), desc).set_emoji(e.name, e.id));
		} else if (!inv.flags.empty() && (inv.flags[0] == 'A' || inv.flags[0] == 'W')) {
			equip_menu.add_select_option(dpp::select_option("Equip " + inv.name, inv.name + ";" + inv.flags + ";" + std::to_string(++index), desc).set_emoji(e.name, e.id));
		}
	}

	if (!drop_menu.options.empty()) {
		m.add_component(dpp::component().add_component(drop_menu));
	}
	if (!use_menu.options.empty()) {
		m.add_component(dpp::component().add_component(use_menu));
	}
	if (!equip_menu.options.empty()) {
		m.add_component(dpp::component().add_component(equip_menu));
	}

	event.reply(event.command.type == dpp::it_application_command ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying inventory:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
			event.reply(dpp::message("Internal error displaying inventory:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```").set_flags(dpp::m_ephemeral));
		}
	});
}

