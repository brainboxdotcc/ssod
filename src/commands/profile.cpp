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
#include <ssod/ssod.h>
#include <ssod/database.h>
#include <ssod/commands/profile.h>
#include <ssod/game_player.h>
#include <gen/emoji.h>
#include <fmt/format.h>

using namespace i18n;

dpp::slashcommand profile_command::register_command(dpp::cluster& bot) {
	return tr(dpp::slashcommand("cmd_profile", "profile_desc", bot.me.id)
		.set_dm_permission(true)
		.add_option(dpp::command_option(dpp::co_string, "opt_user", "user_profile_desc", false)));
}

dpp::task<void> profile_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.owner;
	auto param = event.get_parameter("user");
	std::string user;
	player p;
	bool self{false};
	if (param.index() == 0) {
		if (co_await player_is_live(event)) {
			p = get_live_player(event);
			user = p.name;
			self = true;
		}
	} else {
		user = std::get<std::string>(event.get_parameter("user"));
	}
	auto rs = co_await db::co_query("SELECT * FROM game_users WHERE name = ?", {user});
	if (rs.empty()) {
		event.reply(dpp::message(tr(self ? "NOPROFILE" : "NOSUCHUSER", event)).set_flags(dpp::m_ephemeral));
		co_return;
	}
	p.experience = atol(rs[0].at("experience"));
	auto g = co_await db::co_query("SELECT * FROM guild_members JOIN guilds ON guild_id = guilds.id WHERE user_id = ?", {rs[0].at("user_id")});
	auto status = co_await db::co_query("SELECT passive_effect_status.*, on_end, on_after, type, requirements, duration, withdrawl FROM passive_effect_status join passive_effect_types on passive_effect_id = passive_effect_types.id where user_id = ?", {rs[0].at("user_id")});
	std::stringstream effects;
	for (const auto& s : status) {
		effects << tr(dpp::uppercase(s.at("type")), event) << ": *" << s.at("requirements") << "* ";
		if (s.at("current_state") == "active") {
			effects << tr("ACTIVE_EFF", event) << dpp::utility::timestamp(atoll(s.at("duration")) + atoll(s.at("last_transition_time")), dpp::utility::tf_relative_time);
		} else {
			effects << tr("COOLDOWN_EFF", event) << dpp::utility::timestamp(atoll(s.at("withdrawl")) + atoll(s.at("last_transition_time")), dpp::utility::tf_relative_time);
		}
		effects << "\n";
	}

	std::string content{"### " + tr("LEVEL", event) + " " + std::to_string(p.get_level()) + " " + std::string(race((player_race)atoi(rs[0].at("race")))) + " " + std::string(profession((player_profession)atoi(rs[0].at("profession")))) +  "\n"};
	int percent = p.get_percent_of_current_level();
	for (int x = 0; x < 100; x += 10) {
		if (x < percent) {
			content += sprite::bar_green.get_mention();
		} else {
			content += sprite::bar_red.get_mention();
		}
	}
	content += " (" + std::to_string(percent) + "%)";

	if (!g.empty()) {
		content += "\n\n**" + tr("GUILD", event) + ":** " + dpp::utility::markdown_escape(g[0].at("name"));
	}

	player p2(atol(rs[0].at("user_id")), false);

	std::string file = matrix_image((player_race)atoi(rs[0].at("race")), (player_profession)atoi(rs[0].at("profession")), rs[0].at("gender") == "male");

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title(tr("PROFILE", event) + ": " + dpp::utility::markdown_escape(user))
		.set_footer(dpp::embed_footer{ 
			.text = tr("REQUESTED_BY", event, event.command.usr.format_username()),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(content)
		.set_image(file)
		.add_field(tr("STAMINA", event), sprite::heart.get_mention() + " " + rs[0].at("stamina") + "/" + std::to_string(p2.max_stamina()), true)
		.add_field(tr("SKILL", event), sprite::book07.get_mention() + " " + rs[0].at("skill") + "/" + std::to_string(p2.max_skill()), true)
		.add_field(tr("LUCK", event), sprite::clover.get_mention() + " " + rs[0].at("luck") + "/" + std::to_string(p2.max_luck()), true)
		.add_field("XP", sprite::ac_medal01.get_mention() + " " + rs[0].at("experience"), true)
		.add_field(tr("SPEED", event), sprite::shoes03.get_mention() + " " + rs[0].at("speed") + "/" + std::to_string(p2.max_speed()), true)
		.add_field(tr("SNEAK", event), sprite::throw05.get_mention() + " " + rs[0].at("sneak") + "/" + std::to_string(p2.max_sneak()), true)
		.add_field(tr("GOLD", event), sprite::goldcoin.get_mention() + " " + rs[0].at("gold") + "/" + std::to_string(p2.max_gold()), true)
		.add_field(tr("MANA", event), sprite::hat02.get_mention() + " " + rs[0].at("mana") + "/" + std::to_string(p2.max_mana()), true)
		.add_field(tr("ARMOUR", event), sprite::elm03.get_mention() + " " + rs[0].at("armour_rating") + " (" + rs[0].at("armour") + ")", true)
		.add_field(tr("WEAPON", event), sprite::axe013.get_mention() + " " + rs[0].at("weapon_rating") + " (" + rs[0].at("weapon") + ")", true)
		.add_field(tr("NOTORIETY", event), sprite::elm01.get_mention() + " " + rs[0].at("notoriety"), true)
		.add_field(tr("RATIONS", event), sprite::cheese.get_mention() + " " + rs[0].at("rations") + "/" + std::to_string(p2.max_rations()), true)
		.add_field(tr("SCROLLS", event), sprite::scroll.get_mention() + " " + rs[0].at("scrolls"), true)
	;
	if (rs[0].at("user_id") == event.command.usr.id.str() && !effects.str().empty()) {
		/* Can only view your own status effects */
		embed.add_field(tr("EFFECTS", event), effects.str(), false);
	}

	auto premium = co_await db::co_query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { rs[0].at("user_id") });
	if (!event.command.entitlements.empty() || !premium.empty()) {
		auto bio = co_await db::co_query("SELECT * FROM character_bio WHERE user_id = ?", { rs[0].at("user_id") });
		if (!bio.empty()) {
			if (!bio[0].at("bio").empty()) {
				embed.set_description(content + "\n### " + tr("BIOGRAPHY", event) + "\n" + dpp::utility::markdown_escape(bio[0].at("bio")) + "\n\n");
			}
			if (!bio[0].at("image_name").empty()) {
				embed.set_image("https://premium.ssod.org/profiles/" + bio[0].at("image_name"));
			}
		}
	}
	event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	co_return;
}
