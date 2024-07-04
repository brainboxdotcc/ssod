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

/**
 * @brief Event listeners for DPP events
 */
namespace listeners {
	/**
	 * @brief handle shard ready
	 * 
	 * @param event ready_t
	 */
	dpp::task<void> on_ready(const dpp::ready_t &event);

	/**
	 * @brief handle slash command
	 * 
	 * @param event slashcommand_t
	 */
	void on_slashcommand(const dpp::slashcommand_t& event);

	/**
	 * @brief handle guild join
	 * 
	 * @param event guild_create_t
	 */
	void on_guild_create(const dpp::guild_create_t &event);

	/**
	 * @brief handle guild kick
	 * 
	 * @param event guild_create_t
	 */
	void on_guild_delete(const dpp::guild_delete_t &event);

	/**
	 * @brief Handle creation of a premium entitlement
	 *
	 * @param event
	 */
	void on_entitlement_create(const dpp::entitlement_create_t& event);

	/**
	 * @brief Handle deletion of a premium entitlement
	 *
	 * @param event
	 */
	void on_entitlement_delete(const dpp::entitlement_delete_t& event);

	/**
	 * @brief Handle update of a premium entitlement
	 *
	 * @param event
	 */
	void on_entitlement_update(const dpp::entitlement_update_t& event);

	/**
	 * @brief Return json command definitions
	 * @param bot cluster reference
	 * @return std::string bot command definitions
	 */
	std::string json_commands(dpp::cluster& bot);

};
