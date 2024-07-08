/************************************************************************************
 * 
 * Sporks, the learning, scriptable Discord bot!
 *
 * Copyright 2019 Craig Edwards <support@sporks.gg>
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

#pragma once
#include <dpp/dpp.h>
#include <dpp/json.h>
#include <map>
#include <string>
#include <ssod/ssod.h>
#include <ssod/paragraph.h>
#include <ssod/game_player.h>

namespace js {

	using var_list = std::map<std::string, json>;

	struct script_result {
		bool success{};
		player current_player;
		paragraph p;
	};

	using js_callback = std::function<void(script_result)>;

	void init(class dpp::cluster& _bot);
	bool run(const std::string& script, paragraph& p, player& current_player, const var_list &vars);
	void run(const std::string& script, paragraph& p, player& current_player, const var_list &vars, const js_callback& callback);
	dpp::async<script_result> co_run(const std::string& script, paragraph& p, player& current_player, const std::map<std::string, json> &vars);
}
