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
#include <set>
#include <malloc.h>
#include <fmt/format.h>
#include <ssod/listeners.h>
#include <ssod/database.h>
#include <ssod/combat.h>
#include <ssod/command.h>
#include <ssod/game_player.h>
#include <ssod/lang.h>
#include <ssod/game_util.h>
#include <ssod/achievement.h>

#include <ssod/commands/info.h>
#include <ssod/commands/start.h>
#include <ssod/commands/map.h>
#include <ssod/commands/admin.h>
#include <ssod/commands/lore.h>
#include <ssod/commands/help.h>
#include <ssod/commands/profile.h>
#include <ssod/commands/body.h>
#include <ssod/commands/vote.h>
#include <ssod/commands/bio.h>
#include <ssod/commands/rename.h>
#include <ssod/commands/premium.h>
#include <ssod/commands/reset.h>
#include <ssod/commands/guild.h>
#include <ssod/commands/achievements.h>

#include <ssod/botlist.h>
#include <ssod/botlists/topgg.h>
#include <ssod/botlists/discordbotlist.h>
#include <ssod/botlists/infinitybots.h>
#include <ssod/botlists/voidbots.h>
#include <ssod/botlists/discords.h>
#include <ssod/botlists/botlistme.h>

namespace listeners {

	/**
	 * @brief Welcome a new guild to the bot with some advice on getting started
	 * 
	 * @param bot 
	 * @param guild_id 
	 * @param channel_id 
	 */
	dpp::task<void> send_welcome(dpp::cluster& bot, dpp::snowflake guild_id, dpp::snowflake channel_id) {
		/*bot.message_create(
			dpp::message(channel_id, "")
			.add_embed(
				dpp::embed()
				.set_description("Welcome to the land of __Utopia__! To start your adventure use the `/start` command.\n\nTo read background information about the game world and its people at any time, use the `/lore` command.\n\nYou can get help by joining the [Brainbox.cc Discord](https://discord.gg/brainbox)")
				.set_title("Welcome, Adventurers!")
				.set_color(EMBED_COLOUR)
				.set_url("https://ssod.org/")
				.set_thumbnail(bot.me.get_avatar_url())
				.set_footer("The Seven Spells Of Destruction", bot.me.get_avatar_url())
			)
		);*/
		/* Probably successfully welcomed */
		co_await db::co_query("UPDATE guild_cache SET welcome_sent = 1 WHERE id = ?", {guild_id});
		co_return;
	}

	dpp::task<void> process_potion_drops(dpp::cluster& bot) {
		auto rs = co_await db::co_query("SELECT user_id, origin FROM potion_drops");
		for (const auto& user : rs) {
			dpp::snowflake user_id(user.at("user_id"));
			dpp::interaction_create_t e;
			e.command.usr.id = user_id;
			if (co_await player_is_live(e)) {
				player p = get_live_player(e, false);
				p.pickup_possession(stacked_item{ .name = "skill potion", .flags = "SK+5", .qty = 1});
				p.pickup_possession(stacked_item{ .name = "stamina potion", .flags = "ST+5", .qty = 1});
				p.inv_change = true;
				if (user.at("origin") == "vote") {
					co_await achievement_check("VOTE", e, p, {{"time", time(nullptr)}});
				}
				p.add_toast({ .message = "## A loot drop has arrived!\n\nYou have received a stamina potion and a skill potion!", .image = "potions.jpg" });
				p.last_resurrect = time(nullptr) - 3600;
				update_live_player(p.event, p);
				p.save(user_id);
			}
			co_await db::co_query("DELETE FROM potion_drops WHERE user_id = ?", {user_id});
		}
		co_return;
	}

	/**
	 * @brief Check every 30 seconds for new guilds and welcome them
	 * 
	 * @param bot cluster ref
	 */
	dpp::task<void> welcome_new_guilds(dpp::cluster& bot) {
		auto result = co_await db::co_query("SELECT id FROM guild_cache WHERE welcome_sent = 0");
		for (const auto& row : result) {
			/* Determine the correct channel to send to */
			dpp::snowflake guild_id = row.at("id");
			bot.log(dpp::ll_info, "New guild: " + guild_id.str());
			/* Temp disabled */
			co_await db::co_query("UPDATE guild_cache SET welcome_sent = 1 WHERE id = ?", {guild_id});
			continue;
			auto cc = co_await bot.co_guild_get(guild_id);
			if (cc.is_error()) {
				/* Couldn't fetch the guild - kicked within 30 secs of inviting, bummer. */
				co_await db::co_query("UPDATE guild_cache SET welcome_sent = 1 WHERE id = ?", {guild_id});
				co_return;
			}
			dpp::guild guild = std::get<dpp::guild>(cc.value);
			/* First try to send the message to system channel or safety alerts channel if defined */
			if (!guild.system_channel_id.empty()) {
				co_await send_welcome(bot, guild.id, guild.system_channel_id);
				co_return;
			}
			if (!guild.safety_alerts_channel_id.empty()) {
				co_await send_welcome(bot, guild.id, guild.safety_alerts_channel_id);
				co_return;
			}
			/* As a last resort if they dont have one of those channels set up, find a named
			 * text channel that looks like its the main general/chat channel
			 */
			auto cc2 = co_await bot.co_channels_get(guild_id);
			if (cc2.is_error()) {
				/* Couldn't fetch the channels - kicked within 30 secs of inviting, bummer. */
				co_await db::co_query("UPDATE guild_cache SET welcome_sent = 1 WHERE id = ?", {guild_id});
				co_return;
			}
			dpp::channel_map channels = std::get<dpp::channel_map>(cc.value);
			dpp::snowflake selected_channel_id, first_text_channel_id;
			for (const auto& c : channels) {
				const dpp::channel& channel = c.second;
				std::string lowername = dpp::lowercase(channel.name);
				if ((lowername == "general" || lowername == "chat" || lowername == "moderators") && channel.is_text_channel()) {
					selected_channel_id = channel.id;
					break;
				} else if (channel.is_text_channel()) {
					first_text_channel_id = channel.id;
				}
			}
			if (selected_channel_id.empty() && !first_text_channel_id.empty()) {
				selected_channel_id = first_text_channel_id;
			}
			if (!selected_channel_id.empty()) {
				co_await send_welcome(bot, guild_id, selected_channel_id);
			} else {
				/* What sort of server has NO text channels and invites a game bot??? */
				co_await db::co_query("UPDATE guild_cache SET welcome_sent = 1 WHERE id = ?", {guild_id});
			}
		}
		co_return;
	}

	dpp::task<void> on_entitlement_create(const dpp::entitlement_create_t& event) {
		co_await db::co_query("INSERT INTO premium_credits (user_id, subscription_id, active, since, plan_id, payment_failed, created_at, updated_at)"
			  "VALUES(?, ?, 1, now(), 'ssod-monthly', 0, now(), now()) ON DUPLICATE KEY UPDATE subscription_id = ?, active = 1",
			  { event.created.user_id, event.created.subscription_id, event.created.subscription_id });
	}

	dpp::task<void> on_entitlement_delete(const dpp::entitlement_delete_t& event) {
		co_await db::co_query("UPDATE premium_credits SET active = 0, cancel_date = now(), updated_at = now() WHERE user_id = ? AND subscription_id = ?",
			  { event.deleted.user_id, event.deleted.subscription_id });
	}

	dpp::task<void> on_entitlement_update(const dpp::entitlement_update_t& event) {
		co_await db::co_query("UPDATE premium_credits SET active = ?, updated_at = now() WHERE user_id = ? AND subscription_id = ?",
			  { event.updating_entitlement.is_deleted() || event.updating_entitlement.ends_at < time(nullptr) ? 0 : 1, event.updating_entitlement.user_id, event.updating_entitlement.subscription_id });
	}

	std::vector<dpp::slashcommand> get_commands(dpp::cluster& bot) {
		return {
			register_command<info_command>(bot),
			register_command<start_command>(bot),
			register_command<map_command>(bot),
			register_command<lore_command>(bot),
			register_command<help_command>(bot),
			register_command<profile_command>(bot),
			register_command<body_command>(bot),
			register_command<vote_command>(bot),
			register_command<bio_command>(bot),
			register_command<rename_command>(bot),
			register_command<premium_command>(bot),
			register_command<reset_command>(bot),
			register_command<guild_command>(bot),
			register_command<achievements_command>(bot),
		};
	}

	std::string json_commands(dpp::cluster& bot) {
		json j = json::array();
		std::cerr << "Command List\n";
		auto v = get_commands(bot);
		std::cerr << "Total defined commands: " << v.size() << "\n\n";
		for (auto & s : v) {
			j.push_back(s.to_json(false));
		}
		return j.dump(1, '\t', false, json::error_handler_t::replace);
	}

	dpp::task<void> on_ready(const dpp::ready_t& event) {
		dpp::cluster& bot = *(event.owner);
		if (dpp::run_once<struct register_bot_commands>()) {
			if (bot.cluster_id == 0) {
				bot.global_bulk_command_create(get_commands(bot),
					[&bot](const auto& cc) {
						if (cc.is_error()) {
							bot.log(dpp::ll_error, cc.http_info.body);
						}
						auto slashcommands = std::get<dpp::slashcommand_map>(cc.value);
						bot.log(dpp::ll_info, "Registered " + std::to_string(slashcommands.size()) + " application commands");
					}
				);
				bot.guild_bulk_command_create({
					register_command<admin_command>(bot),
				}, 537746810471448576);
			}

			auto set_presence = [&bot]() {
				auto rs =db::query("SELECT (SELECT COUNT(id) FROM guild_cache) AS guild_count, (SELECT SUM(user_count) FROM guild_cache) AS discord_user_count, (SELECT COUNT(user_id) FROM game_users) AS game_user_count");
				bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_game, fmt::format("on {} servers with {} active players and {} users", rs[0].at("guild_count"), rs[0].at("game_user_count"), rs[0].at("discord_user_count"))));
			};

			bot.start_timer([set_presence](dpp::timer t) {
				set_presence();
			}, 240);
			bot.start_timer([&bot](dpp::timer t) {
				post_botlists(bot).sync_wait();
			}, 60 * 15);
			bot.start_timer([&bot](dpp::timer t) {
				welcome_new_guilds(bot).sync_wait();
			}, 30);
			bot.start_timer([](dpp::timer t) {
				end_abandoned_pvp().sync_wait();
			}, 10);
			bot.start_timer([&bot](dpp::timer t) {
				process_potion_drops(bot).sync_wait();
				cleanup_idle_live_players().sync_wait();
				i18n::check_lang_reload(bot);
			}, 60);
			bot.start_timer([](dpp::timer t) {
				/* Garbage collect free memory by consolidating free malloc() blocks */
				malloc_trim(0);
			}, 600);
			bot.start_timer([&bot](dpp::timer t) {
				check_effects(bot).sync_wait();
			}, 1);

			set_presence();
			co_await welcome_new_guilds(bot);
			co_await process_potion_drops(bot);

			register_botlist<topgg>();
			register_botlist<discordbotlist>();
			register_botlist<infinitybots>();
			register_botlist<voidbots>();
			register_botlist<discords>();
			register_botlist<botlistme>();

			co_await post_botlists(bot);
		}
		co_return;
	}

	dpp::task<void> on_guild_create(const dpp::guild_create_t &event) {
		if (event.created.is_unavailable()) {
			co_return;
		}
		co_await db::co_query("INSERT INTO guild_cache (id, owner_id, name, user_count) VALUES(?,?,?,?) ON DUPLICATE KEY UPDATE owner_id = ?, name = ?, user_count = ?", { event.created.id, event.created.owner_id, event.created.name, event.created.member_count, event.created.owner_id, event.created.name, event.created.member_count });
	}

	dpp::task<void> on_guild_delete(const dpp::guild_delete_t &event) {
		if (!event.deleted.is_unavailable()) {
			co_await db::co_query("DELETE FROM guild_cache WHERE id = ?", { event.deleted.id });
			event.owner->log(dpp::ll_info, "Removed from guild: " + event.deleted.id.str());
		}
		co_return;
	}

	dpp::task<void> on_slashcommand(const dpp::slashcommand_t &event) {
		double start = dpp::utility::time_f();
		co_await route_command(event);
		double secs = dpp::utility::time_f() - start;
		event.owner->log(
			dpp::ll_info,
			fmt::format(
				"COMMAND: {} by {} ({} Guild: {}) Locale: {}, msecs: {:.02f}",
				event.command.get_command_name(),
				event.command.usr.format_username(),
				event.command.usr.id,
				event.command.guild_id,
				event.command.locale,
				secs * 1000
			)
		);
		if (secs > 3.0) {
			event.owner->log(dpp::ll_warning, "Command response took > 3 seconds!");
		}
		co_return;
	}
}
