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

#include <ssod/ssod.h>
#include <ssod/game_player.h>

namespace quests {

	/**
 * Struct representing metadata for a quest.
 */
	struct quest_metadata {
		std::string title;      ///< Title of the quest
		long total_steps = 0;   ///< Total number of quest steps
	};

	/**
	 * Fetch metadata for a quest by its ID.
	 *
	 * @param quest_id The ID of the quest.
	 * @return A quest_metadata struct. If the quest is not found, fields are default-initialised.
	 */
	dpp::task<quest_metadata> get_quest_metadata(long quest_id);

	/**
	 * @brief Automatically begins any quest whose trigger paragraph has just been crossed.
	 *
	 * This enables a CYOA-style experience with minimal manual tracking: players can begin
	 * quests purely by reaching relevant locations, which is critical for fluid and immersive
	 * progression. This is intended to be called after each paragraph transition.
	 *
	 * @param p The player state object (inventory, stats, flags, etc).
	 * @param event Discord interaction, used to extract the user ID.
	 * @return A coroutine that completes after inserting any new quest progress entries.
	 */
	dpp::task<void> autostart_if_needed(player& p, const dpp::interaction_create_t& event);

	/**
	 * @brief Evaluates whether any in-progress quest should be advanced by one step.
	 *
	 * This checks the current step of each active quest and determines whether its
	 * goals have been fulfilled (e.g., item collected, flag set, stat reached).
	 * If so, it advances the quest to the next step. Called before each save of the player.
	 *
	 * @param p The player object containing current stats and inventory.
	 * @param event Discord interaction, from which the user ID is derived.
	 * @return A coroutine that completes once all quests have been evaluated.
	 */
	dpp::task<void> evaluate_all(player& p, const dpp::interaction_create_t& event);

	/**
	 * @brief Determines whether a specific quest step has been completed.
	 *
	 * Used to gate progression between quest steps. This checks all required goals
	 * (inventory, stat minimums, flags, paragraph reached) for the step and verifies
	 * that all non-optional goals are met.
	 *
	 * @param p The player in question.
	 * @param quest_id The database ID of the quest.
	 * @param step_index The 0-based index of the step to evaluate.
	 * @return A coroutine resolving to true if the step is complete, false otherwise.
	 */
	dpp::task<bool> step_is_complete(player& p, long quest_id, long step_index);

	/**
	 * @brief Checks if a single goal condition has been met.
	 *
	 * Supports flags, paragraph IDs, numeric stats, and inventory items by name.
	 * This decouples logic from C++ and ensures that the entire quest system is
	 * driven by database configuration alone.
	 *
	 * @param p The player to check.
	 * @param val The string value identifying the goal (flag, item name, stat name, or paragraph ID).
	 * @param amount The numeric threshold required (e.g., number of items, gold, scrolls).
	 * @return True if the player meets the goal, false otherwise.
	 */
	bool goal_met(player& p, const std::string& val, long amount);

	dpp::task<void> continue_quest_log(const dpp::interaction_create_t& event, player &p);

	dpp::task<bool> quest_log_nav(const dpp::interaction_create_t& event, player &p, const std::vector<std::string>& parts);

}
