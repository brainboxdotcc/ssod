/************************************************************************************
 * 
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023 Craig Edwards <support@sporks.gg>
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
#include <ssod/commands/info.h>
#include <ssod/database.h>
#include <ssod/sentry.h>
#include <ssod/game_date.h>

dpp::slashcommand info_command::register_command(dpp::cluster& bot)
{
	return dpp::slashcommand("info", "Show bot information", bot.me.id);
}

int64_t proc_self_value(const std::string& find_token) {
	int64_t ret = 0;
	std::ifstream self_status("/proc/self/status");
	while (self_status) {
		std::string token;
		self_status >> token;
		if (token == find_token) {
			self_status >> ret;
			break;
		}
	}
	self_status.close();
	return ret;
}

int64_t rss() {
	return proc_self_value("VmRSS:") * 1024;
}

bool is_gdb() {
	return proc_self_value("TracerPid:") != 0;
}

void info_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;
	bot->current_application_get([bot, event](const dpp::confirmation_callback_t& v) {
		dpp::application app;
		uint64_t guild_count = 0;
		if (!v.is_error()) {
			app = std::get<dpp::application>(v.value);
			guild_count = app.approximate_guild_count;
		}
		dpp::embed embed = dpp::embed()
			.set_url("https://ssod.org/")
			.set_title("The Seven Spells Of Destruction")
			.set_footer(dpp::embed_footer{ 
				.text = "Requested by " + event.command.usr.format_username(), 
				.icon_url = bot->me.get_avatar_url(), 
				.proxy_url = "",
			})
			.set_colour(0x7aff7a)
			.set_description("")
			.add_field("Bot Uptime", bot->uptime().to_string(), true)
			.add_field("Memory Usage", std::to_string(rss() / 1024 / 1024) + "M", true)
			.add_field("Total Servers", std::to_string(guild_count), true)
			.add_field("Sentry Version", sentry::version(), true)
			.add_field("Log Queue Length", std::to_string(sentry::queue_length()), true)
			.add_field("Debugging", is_gdb() ? ":white_check_mark: Yes" : "<:wc_rs:1174363531794202624> No", true)
			.add_field("Guild Members Intent", ":white_check_mark: Yes", true)
			.add_field("Message Content Intent", "<:wc_rs:1174363531794202624> No", true)
			.add_field("Shard", std::to_string(event.from->shard_id) + "/" + std::to_string(bot->get_shards().size()), true)
			.add_field("SQL cache size", std::to_string(db::cache_size()), true)
			.add_field("SQL query count", std::to_string(db::query_count()), true)
			.add_field("Game Time", game_date(), false)
			;

		embed.add_field("Library Version", "<:DPP1:847152435399360583><:DPP2:847152435343523881> [" + std::string(DPP_VERSION_TEXT) + "](https://dpp.dev/)", false);

		event.reply(dpp::message().add_embed(embed));
	});
}
