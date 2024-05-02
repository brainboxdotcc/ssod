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
#include <ssod/commands/map.h>
#include <fmt/format.h>

dpp::slashcommand map_command::register_command(dpp::cluster& bot)
{
	return _(dpp::slashcommand("cmd_map", "map_desc", bot.me.id).set_dm_permission(true));
}

void map_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title(_("MAP", event))
		.set_footer(dpp::embed_footer{ 
			.text = _("REQUESTED_BY", event, event.command.usr.format_username()),
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_image("https://images.ssod.org/resource/map.jpg")
		.set_description("");
	event.reply(dpp::message()
		.add_embed(embed)
		.set_flags(dpp::m_ephemeral)
	);
}
