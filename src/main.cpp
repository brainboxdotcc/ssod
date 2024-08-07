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
#include <ssod/listeners.h>
#include <ssod/database.h>
#include <ssod/logger.h>
#include <ssod/config.h>
#include <ssod/game.h>
#include <ssod/js.h>
#include <ssod/aes.h>
#include <ssod/commandline.h>
#include <ssod/lang.h>
#include <ssod/sentry.h>

int main(int argc, char const *argv[]) {
	std::setlocale(LC_ALL, "en_GB.UTF-8");

	config::init("../config.json");
	logger::init(config::get("log"));
	commandline_config cli = commandline::parse(argc, argv);

	dpp::cluster bot(
		config::get("token"),
		dpp::i_guilds,
		config::get("shards"),
		cli.cluster_id,
		cli.max_clusters,
		true,
		dpp::cache_policy::cpol_none
	);

	sentry::init(bot);
	i18n::load_lang(bot);

	if (cli.display_commands) {
		std::cerr << listeners::json_commands(bot) << "\n";
		exit(0);
	}

	bot.set_websocket_protocol(dpp::ws_etf);

	security::init(bot);
	js::init(
		bot,
		config::exists("js_thread_pool_size") ?
		config::get("js_thread_pool_size").get<int>() :
		std::thread::hardware_concurrency()
	);

	bot.on_log(&logger::log);
	bot.on_guild_create(&listeners::on_guild_create);
	bot.on_guild_delete(&listeners::on_guild_delete);
	bot.on_slashcommand(&listeners::on_slashcommand);
	bot.on_entitlement_create(&listeners::on_entitlement_create);
	bot.on_entitlement_update(&listeners::on_entitlement_update);
	bot.on_entitlement_delete(&listeners::on_entitlement_delete);
	bot.on_button_click(&game_nav);
	bot.on_select_click(&game_select);
	bot.on_form_submit(&game_input);
	bot.on_ready(&listeners::on_ready);

	db::init(bot);

	/* Start bot */
	bot.start(dpp::st_wait);
}

