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

std::string pretty_duration(uint64_t diff) {
	if (diff == 0) {
		return "none";
	}
	auto const secs = diff % 60;
	diff /= 60;
	auto const mins = diff % 60;
	diff /= 60;
	auto const hours = diff % 24;
	diff /= 24;
	auto const days = diff;
	std::string r =
		std::string(days ? (std::to_string(days) + " day" + std::string(days > 1 ? "s" : "") + ", ") : "") +
		std::string(hours ? (std::to_string(hours) + " hour" + std::string(hours > 1 ? "s" : "") + ", ") : "") +
		std::string(mins ? (std::to_string(mins) + " minute" + std::string(mins > 1 ? "s" : "") + ", ") : "") +
		std::string(secs ? (std::to_string(secs) + " second" + std::string(secs > 1 ? "s" : "") + ", ") : "");
	return r.substr(0, r.length() - 2);
}

void grimoire(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	std::stringstream content;

	std::vector<dpp::embed_field> fields;

	content << "__**" << tr("SPELLS", event) << "**__ - " + tr("MANA_CHARGE", event, p.profession == prof_wizard ? 2 : 1) + "\n";
	content << sprite::hat02.get_mention() << " " << tr("MANA", event) << ": __" << p.mana << "/" << p.max_mana() << "__\n";

	dpp::component cast_menu;
	cast_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder(tr("CASTONE", event))
		.set_id(security::encrypt("cast"));

	std::ranges::sort(p.spells, [](const item &a, const item &b) -> bool { return a.name < b.name; });
	uint32_t index{0};
	for (const auto &inv: p.spells) {
		auto rs = db::query("SELECT * FROM passive_effect_types WHERE type = 'Spell' AND requirements = ? AND (SELECT COUNT(*) FROM passive_effect_status WHERE user_id = ? AND passive_effect_id = passive_effect_types.id) = 0", {inv.name, event.command.usr.id});
		if (!rs.empty()) {
			spell_info si = get_spell_info(inv.name);
			std::string duration = pretty_duration(atoll(rs[0].at("duration")));
			std::string cooldown = pretty_duration(atoll(rs[0].at("withdrawl")));
			std::string sn = human_readable_spell_name(inv.name, event);
			if (p.mana >= si.mana_cost) {
				cast_menu.add_select_option(dpp::select_option(tr("CAST", event, sn), inv.name + ";" + std::to_string(++index)).set_emoji("🪄"));
			}
			fields.emplace_back(
				"🪄 " + sn,
				"```ansi\n\033[2;36mDuration\033[0m \033[2;34m" + duration +
				"\n\033[2;36mCooldown\033[0m \033[2;34m" + cooldown +
				"\n\033[2;36mMana Cost\033[0m \033[2;34m" + std::to_string(si.mana_cost) +
				"\n```",
			true);
		}

	}

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{
			.text = tr("GRIMOIREFOOTER", event, p.name),
			.icon_url = bot.me.get_avatar_url(),
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(content.str());

	dpp::message m;
	component_builder cb(m);

	cb.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id(security::encrypt("exit_grimoire"))
		.set_label(tr("BACK", event))
		.set_style(dpp::cos_primary)
		.set_emoji(sprite::magic05.name, sprite::magic05.id)
	);
	cb.add_component(help_button(event));

	m = cb.get_message();

	if (!cast_menu.options.empty()) {
		m.add_component(dpp::component().add_component(cast_menu));
	}

	embed.fields = fields;
	m.embeds = { embed };

	event.reply(event.command.type == dpp::it_application_command ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying grimoire:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
			event.reply(dpp::message("Internal error displaying grimoire:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```").set_flags(dpp::m_ephemeral));
		}
	});
}

