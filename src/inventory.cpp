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
#include <ssod/database.h>

using namespace i18n;

void inventory(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	std::stringstream content;
	bool equip_w{false}, equip_a{false};

	size_t pages_max = ceil(p.max_inventory_slots() / 25);
	if (p.inventory_page > pages_max) {
		p.inventory_page = 0;
	}

	std::vector<dpp::embed_field> fields;

	content << "__**" << tr("Stats", event) << "**__\n";
	content << "<:" << sprite::health_heart.format() << "> " << tr("STAMINA", event) << ": __" << p.stamina << "__";
	content << " <:" << sprite::book07.format() << "> " << tr("SKILL", event) << ": __" << p.skill << "__";
	content << " <:" << sprite::clover.format() << "> " << tr("LUCK", event) << ": __" << p.luck << "__";
	content << " <:" << sprite::medal01.format() << "> XP: __" << p.experience << "__ (" << tr("LEVEL", event) << ": __" << p.get_level() << "__)\n";
	content << " <:" << sprite::shoes03.format() << "> " << tr("SPEED", event) << ": __" << p.speed << "__";
	if (p.gold > 0) {
		content << " <:" << sprite::gold_coin.format() << "> " << tr("GOLD", event) << ": __" << p.gold << "__";
	}
	if (p.silver > 0) {
		content << " <:" << sprite::silver_coin.format() << "> " << tr("SILVER", event) << ": __" << p.silver << "__";
	}

	content << "\n";
	if (p.has_flag("pack") || p.has_flag("horse") || p.has_flag("steamcopter") || p.has_flag("saddlebags")) {
		content << "\n__**" << tr("STORAGE", event) << "**__\n";
		if (p.has_flag("pack")) {
			content << sprite::backpack.get_mention() << " " << tr("BACKPACK", event) << " (__+1__) ";
		}
		if (p.has_flag("horse")) {
			content << "🐴 " << tr("HORSE", event) << " (__+1__) ";
		}
		if (p.has_flag("saddlebags")) {
			content << sprite::backpack.get_mention() << " " << tr("SADDLEBAGS", event) << " (__+1__) ";
		}
		if (p.has_flag("steamcopter")) {
			content << "🚁 " << tr("STEAMCOPTER", event) << " (__+3__) ";
		}
		content << "\n";
	}

	content << "\n__**" << tr("SPELLS", event) << "**__\n";
	std::ranges::sort(p.spells, [](const item &a, const item &b) -> bool { return a.name < b.name; });
	for (const auto &inv: p.spells) {
		content << "🪄 " << human_readable_spell_name(inv.name, event) << "\n";
	}
	content << "\n__**" << tr("HERBS", event) << "**__\n";
	std::ranges::sort(p.herbs, [](const item &a, const item &b) -> bool { return a.name < b.name; });
	for (const auto &inv: p.herbs) {
		content << "🌿 " << human_readable_herb_name(inv.name, event) << "\n";
	}

	content << "\n__**" << tr("INVENTORYPAGE", event, p.inventory_page + 1, pages_max++) << "**__\n";

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{
			.text = tr("INVENTORYFOOTER", event, p.name, p.inventory_page + 1),
			.icon_url = bot.me.get_avatar_url(),
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(content.str());

	dpp::message m;

	std::vector<stacked_item> possessions = p.possessions_page(p.inventory_page);
	std::ranges::sort(possessions, [](const stacked_item &a, const stacked_item& b) -> bool { return a.name < b.name; });

	component_builder cb(m);
	size_t index{0};

	cb.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id(security::encrypt("exit_inventory"))
		.set_label(tr("BACK", event))
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
	cb.add_component(help_button(event));

	m = cb.get_message();

	dpp::component use_menu, drop_menu, equip_menu/*, examine_menu*/;

	use_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder(tr("USEITEM", event))
		.set_id(security::encrypt("use_item"));
	drop_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder(tr("DROPITEM", event))
		.set_id(security::encrypt("drop_item"));
	equip_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder(tr("EQUIPITEM", event))
		.set_id(security::encrypt("equip_item"));

	std::set<std::string> dup;
	for (const auto& inv : possessions) {
		sale_info value = get_sale_info(inv.name);
		dpp::emoji e = get_emoji(inv.name, inv.flags);
		auto effect = db::query("SELECT * FROM passive_effect_types WHERE type = 'Consumable' AND requirements = ?", {inv.name});
		auto food = db::query("SELECT * FROM food WHERE name = ?", {inv.name});
		if (!value.quest_item && value.sellable) {
			auto i = tr(inv, "", event);
			drop_menu.add_select_option(dpp::select_option(tr("DROP", event) + " " + i.name, inv.name + ";" + inv.flags + ";" + std::to_string(++index)).set_emoji("❌"));
		}
		if (!food.empty() || !effect.empty() || inv.flags.find('+') != std::string::npos || inv.flags.find('-') != std::string::npos) {
			auto i = tr(inv, "", event);
			use_menu.add_select_option(dpp::select_option(tr("USE", event) + " " + i.name, inv.name + ";" + inv.flags + ";" + std::to_string(++index)).set_emoji(e.name, e.id));
		} else if (!inv.flags.empty() && (inv.flags[0] == 'A' || inv.flags[0] == 'W')) {
			auto i = tr(inv, "", event);
			equip_menu.add_select_option(dpp::select_option(tr("EQUIP", event) + " " + i.name, inv.name + ";" + inv.flags + ";" + std::to_string(++index)).set_emoji(e.name, e.id));
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

	if (possessions.empty()) {
		embed.fields = fields;
		m.embeds = { embed };
	} else {
		size_t c{content.str().length()};
		for (const auto &inv: possessions) {
			std::string emoji = get_emoji(inv.name, inv.flags).format();
			std::string description{"```ansi\n" + describe_item(inv.flags, inv.name, event, true, 80) + "\n"};
			if (p.armour.name == inv.name && !equip_a) {
				description += "\033[2;31m🫱🏼 " + tr("EQUIPPED", event) + "\033[0m ";
				equip_a = true;
			} else if (p.weapon.name == inv.name && !equip_w) {
				description += "\033[2;31m🫱🏼 " + tr("EQUIPPED", event) + "\033[0m ";
				equip_w = true;
			}
			sale_info value = get_sale_info(inv.name);
			if (value.quest_item) {
				description += "\033[2;32m❗ " + tr("QUESTITEM", event) + "\033[0m ";
			}
			if (value.sellable && !value.quest_item) {
				description += "\033[2;33m🪙 " + std::to_string(value.value) + " " + tr("VALUE2", event) + "\033[0m ";
			}
			description += "\n```\n";
			auto i = tr(inv, "", event);
			auto f = dpp::embed_field("<:" + emoji + "> " + i.name + (inv.qty > 1 ? fmt::format(" (x{})", inv.qty) : ""), description, true);
			fields.push_back(f);

			dpp::message saved = m;
			embed.fields = fields;
			m.embeds = { embed };
			c += dpp::utility::utf8len(f.name) + dpp::utility::utf8len(f.value);
			if (c > 6000) {
				/* Check the inventory is not too big to view */
				m = saved;
				bot.log(dpp::ll_warning, "Inventory page too big for discord, ended render at " + std::to_string(m.build_json().length()) + " characters after index " + std::to_string(c));
				break;
			}
			++c;
		}
	}

	event.reply(event.command.type == dpp::it_application_command ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying inventory:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
			event.reply(dpp::message("Internal error displaying inventory:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```").set_flags(dpp::m_ephemeral));
		}
	});
}

