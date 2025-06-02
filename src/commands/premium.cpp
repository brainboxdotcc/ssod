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
#include <ssod/aes.h>
#include <fmt/format.h>

using namespace i18n;

dpp::slashcommand premium_command::register_command(dpp::cluster& bot)
{
	return tr(dpp::slashcommand("cmd_premium", "premium_desc", bot.me.id)
		.set_interaction_contexts({dpp::itc_guild, dpp::itc_bot_dm, dpp::itc_private_channel}));
}

dpp::task<void> premium_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.owner;
	auto rs = co_await db::co_query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { event.command.usr.id });
	bool has_premium = !rs.empty();
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title(tr("SSPREMIUM", event))
		.set_footer(dpp::embed_footer{ 
			.text = tr("REQUESTED_BY", event, event.command.usr.format_username()),
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(!has_premium ? tr("UPSELL", event, event.command.usr.id.str()) : tr("THANKS", event));
		
	event.reply(dpp::message()
		.add_component(
			has_premium ? 
			dpp::component().add_component(
				dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_premium_manage"))
				.set_label(tr("MANAGESUB", event))
				.set_url("https://premium.ssod.org/manage-subscription?user=" + event.command.usr.id.str())
				.set_style(dpp::cos_link)
			)
			:
			dpp::component().add_component(
				dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_premium_cta"))
				.set_label(tr("GETPREM", event))
				.set_url("https://premium.ssod.org/?user=" + event.command.usr.id.str())
				.set_style(dpp::cos_link)
			)	
		)
		.add_embed(embed)
		.set_flags(dpp::m_ephemeral)
	);
	co_return;
}
