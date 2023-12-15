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
#include <ssod/commands/admin.h>
#include <ssod/database.h>
#include <ssod/sentry.h>
#include <ssod/game_date.h>

dpp::slashcommand admin_command::register_command(dpp::cluster& bot)
{
	return dpp::slashcommand("admin", "Game Moderation Commands", bot.me.id)
                .add_option(dpp::command_option(dpp::co_sub_command, "teleport", "Teleport yourself to new location ID"))
                .add_option(dpp::command_option(dpp::co_sub_command, "mute", "Mute a user"))
                .add_option(dpp::command_option(dpp::co_sub_command, "pin", "Pin a user"))
                .add_option(dpp::command_option(dpp::co_sub_command, "reset", "Reset a user"))
		;

}

void admin_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;
	event.reply("This command is for game admins only");
}
