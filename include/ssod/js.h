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

#pragma once
#include <dpp/dpp.h>
#include <dpp/json.h>
#include <map>
#include <string>
#include <ssod/ssod.h>
#include <ssod/paragraph.h>
#include <ssod/game_player.h>

/**
 * @brief This namespace contains functions which relate to the duktape javascript
 * interpreter. The duktape interpreter is wrapped with an asynchronous thread pool
 * which means you can co_await js::co_run and it will resume later when the script
 * has finished within the thread pool. The thread pool size can be set via js::init
 * and will default to the same size a there are logical cores.
 */
namespace js {

	/**
	 * @brief A list of variables to instantiate in the javascript environment
	 */
	using var_list = std::map<std::string, json>;

	/**
	 * @brief The result of a javascript execution, new player and paragraph content
	 */
	struct script_result {
		/**
		 * @brief True if script didn't error
		 */
		bool success{};
		/**
		 * @brief New player data, may be mutated by script
		 */
		player current_player;
		/**
		 * @brief New paragraph data, may be mutated by script
		 */
		paragraph p;
	};

	/**
	 * @brief A javascript execution callback, for asynchronous execution of js scripts
	 */
	using js_callback = std::function<void(script_result)>;

	/**
	 * @brief Initialise duktape interpreter, and thread pool
	 * @param _bot dpp cluster instance
	 * @param thread_pool_size size of thread pool, defaults to same as number of cores
	 */
	void init(class dpp::cluster& _bot, int thread_pool_size = std::thread::hardware_concurrency());

	/**
	 * @brief Execute blocking js script
	 *
	 * @param script Script body
	 * @param p current paragraph
	 * @param current_player current player
	 * @param vars variables to create in the instance
	 * @return true on success
	 */
	bool run(const std::string& script, paragraph& p, player& current_player, const var_list &vars);

	/**
	 * @brief Execute async js script with callback on completion, placing the execution into
	 * the thread pool.
	 *
	 * @param script Script body
	 * @param p current paragraph
	 * @param current_player current player
	 * @param vars variables to create in the instance
	 * @param callback Callback on completion of js script
	 */
	void run(const std::string& script, paragraph& p, player& current_player, const var_list &vars, const js_callback& callback);

	/**
	 * @brief Execute async js script with callback on completion, placing the execution into
	 * the thread pool. As this returns dpp::task<> it may be co_awaited for a script_result.
	 *
	 * @param script Script body
	 * @param p current paragraph
	 * @param current_player current player
	 * @param vars variables to create in the instance
	 * @return dpp::async to be co_awaited for script completion. co_await for script_result.
	 */
	dpp::async<script_result> co_run(const std::string& script, paragraph& p, player& current_player, const var_list& vars);
}
