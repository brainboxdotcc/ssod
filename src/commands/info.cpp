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
#include <ssod/commands/info.h>
#include <ssod/database.h>
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
	auto rs = db::query("SELECT COUNT(id) AS guild_count, SUM(user_count) AS user_count FROM guild_cache");
	std::string hash(GIT_COMMIT_HASH);
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("The Seven Spells Of Destruction")
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_description("")
		.add_field("Bot Uptime", bot->uptime().to_string(), true)
		.add_field("Memory Usage", std::to_string(rss() / 1024 / 1024) + "M", true)
		.add_field("Total Servers", rs[0].at("guild_count"), true)
		.add_field("Git Revision", GIT_BRANCH "@" + hash.substr(hash.length() - 8), true)
		.add_field("Total Users", rs[0].at("user_count"), true)
		.add_field("Cluster", std::to_string(bot->cluster_id) + "/" + std::to_string(bot->maxclusters), true)
		.add_field("Shard", std::to_string(event.from->shard_id) + "/" + std::to_string(bot->get_shards().size()), true)
		.add_field("SQL cache size", std::to_string(db::cache_size()), true)
		.add_field("SQL query count", std::to_string(db::query_count()), true)
		.add_field("Game Time", game_date(), false)
		.set_image("attachment://app_encyclopaedia.jpg")
		;

	embed.add_field("Library Version", "<:DPP1:847152435399360583><:DPP2:847152435343523881> [" + std::string(DPP_VERSION_TEXT) + "](https://dpp.dev/)", false);

	event.reply(dpp::message().add_embed(embed).add_file("app_encyclopaedia.jpg", dpp::utility::read_file("../resource/app_encyclopaedia.jpg")));
}
