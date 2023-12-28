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
#include <dpp/dpp.h>
#include <fmt/format.h>
#include <ssod/ssod.h>
#include <ssod/listeners.h>
#include <ssod/database.h>
#include <ssod/logger.h>
#include <ssod/config.h>
#include <ssod/game.h>

int main(int argc, char const *argv[])
{
	config::init("../config.json");
	logger::init(config::get("log"));

	dpp::cluster bot(
		config::get("token"),
		dpp::i_guilds,
		0, 0, 1, true, dpp::cache_policy::cpol_none
	);

	bot.on_log(&logger::log);
	bot.on_guild_create(&listeners::on_guild_create);
	bot.on_guild_delete(&listeners::on_guild_delete);
	bot.on_slashcommand(&listeners::on_slashcommand);
	bot.on_button_click(&game_nav);
	bot.on_select_click(&game_select);
	bot.on_form_submit(&game_input);
	bot.on_ready(&listeners::on_ready);

	db::init(bot);

	/* Start bot */
	bot.start(dpp::st_wait);
}

