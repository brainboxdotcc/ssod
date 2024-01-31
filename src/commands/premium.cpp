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
#include <ssod/commands/premium.h>
#include <ssod/database.h>
#include <ssod/aes.h>

dpp::slashcommand premium_command::register_command(dpp::cluster& bot)
{
	return dpp::slashcommand("premium", "Provide a link to manage your premium subscription or to subscribe", bot.me.id);
}

void premium_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;
	auto rs = db::query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { event.command.usr.id });
	bool has_premium = rs.empty();
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("Seven Spells Premium")
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_description(
			!has_premium ? "You do not yet have [Seven Spells Premium](https://premium.ssod.org?user=" + event.command.usr.id.str() +
			") Subscriptions are just £3 a month and give access to additional areas, automatic loot drops, and more!\n\nIf you had premium, this command would give you a link to manage your subscription."
			:
			"Thank you for being a **Seven Spells Premium** subscriber! Click the button below to manage your subscription."
		);
		
	event.reply(dpp::message()
		.add_component(
			has_premium ? 
			dpp::component().add_component(
				dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_premium_manage"))
				.set_label("Manage Subscription")
				.set_url("https://premium.ssod.org/manage-subscription?user=" + event.command.usr.id.str())
				.set_style(dpp::cos_link)
			)
			:
			dpp::component().add_component(
				dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_premium_cta"))
				.set_label("Get Premium")
				.set_url("https://premium.ssod.org/?user=" + event.command.usr.id.str())
				.set_style(dpp::cos_link)
			)	
		)
		.add_embed(embed)
		.set_flags(dpp::m_ephemeral)
	);
}
