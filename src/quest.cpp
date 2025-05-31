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
#include <ssod/game_player.h>
#include <ssod/database.h>
#include <ssod/wildcard.h>
#include <ssod/quest.h>

using namespace i18n;

namespace quests {

	dpp::task<quest_metadata> get_quest_metadata(long quest_id) {
		auto rows = co_await db::co_query(
			"SELECT title, (SELECT COUNT(*) FROM quest_steps WHERE quest_id = ?) AS total_steps "
			"FROM quests WHERE id = ?",
			{ quest_id, quest_id }
		);

		if (rows.empty()) {
			co_return quest_metadata{};
		}

		co_return quest_metadata{
			.title = rows[0].at("title"),
			.total_steps = atol(rows[0].at("total_steps"))
		};
	}


	dpp::task<void> autostart_if_needed(player& p, const dpp::interaction_create_t& event) {
		dpp::snowflake user_id = event.command.usr.id;

		auto results = co_await db::co_query(
			"SELECT id FROM quests WHERE JSON_CONTAINS(start_paragraphs, ?)",
			{ "\"" + std::to_string(p.paragraph) + "\"" }
		);

		for (const auto& row : results) {
			int quest_id = std::stoi(row.at("id"));

			auto existing = co_await db::co_query(
				"SELECT 1 FROM quest_progress WHERE user_id = ? AND quest_id = ?",
				{ user_id, quest_id }
			);

			if (existing.empty()) {
				co_await db::co_query(
					"INSERT INTO quest_progress (user_id, quest_id, step_index, status) VALUES (?, ?, 0, 'in_progress')",
					{ user_id, quest_id }
				);
				auto [title, total_steps] = co_await get_quest_metadata(quest_id);
				p.add_toast({
					.message = tr("QUEST_STARTED", event, title),
					.image = "quest-start.png"
				});
			}
		}

		co_return;
	}

	dpp::task<void> evaluate_all(player& p, const dpp::interaction_create_t& event) {
		dpp::snowflake user_id = event.command.usr.id;

		auto rows = co_await db::co_query(
			"SELECT quest_id, step_index FROM quest_progress WHERE user_id = ? AND status = 'in_progress'",
			{ user_id }
		);

		for (const auto& row : rows) {
			long quest_id = atol(row.at("quest_id"));
			long step_index = atol(row.at("step_index"));

			if (co_await step_is_complete(p, quest_id, step_index)) {
				auto meta = co_await get_quest_metadata(quest_id);

				if (step_index + 1 >= meta.total_steps) {
					// Final step completed, mark quest as done
					co_await db::co_query(
						"UPDATE quest_progress SET step_index = ?, status = 'completed' WHERE user_id = ? AND quest_id = ?",
						{ step_index + 1, user_id, quest_id }
					);
					p.add_toast({
						.message = tr("QUEST_COMPLETE", event, meta.title),
						.image = "quest-complete.png"
					});
				} else {
					// Just move to the next step
					co_await db::co_query(
						"UPDATE quest_progress SET step_index = step_index + 1 WHERE user_id = ? AND quest_id = ?",
						{ user_id, quest_id }
					);
					std::string success_text;
					auto step_result = co_await db::co_query(
						"SELECT success_text FROM quest_steps WHERE quest_id = ? AND step_order = ?",
						{ quest_id, step_index }
					);
					if (!step_result.empty()) {
						success_text = step_result[0].at("success_text");
					}
					p.add_toast({
						.message = tr("QUEST_ADVANCE", event, meta.title, success_text),
						.image = "quest-advance.png"
					});
				}
			}
		}

		co_return;
	}


	dpp::task<bool> step_is_complete(player& p, long quest_id, long step_index) {
		auto step_rows = co_await db::co_query(
			"SELECT id FROM quest_steps WHERE quest_id = ? AND step_order = ?",
			{ quest_id, step_index }
		);

		if (step_rows.empty()) {
			co_return false;
		}

		int step_id = std::stoi(step_rows[0].at("id"));

		auto goal_rows = co_await db::co_query(
			"SELECT value, condition_value, optional FROM quest_step_goals WHERE quest_step_id = ?",
			{ step_id }
		);

		for (const auto& goal : goal_rows) {
			const std::string& value = goal.at("value");
			int required = atol(goal.at("condition_value"));
			bool optional = (goal.at("optional") == "1");

			if (!goal_met(p, value, required) && !optional) {
				co_return false;
			}
		}

		co_return true;
	}

	bool goal_met(player& p, const std::string& val, long amount) {

		const std::map<std::string, long> scorename_map = {
			{ "exp", p.experience },
			{ "dice", p.g_dice },
			{ "stm", p.stamina },
			{ "skl", p.skill },
			{ "arm", p.armour.rating },
			{ "wpn", p.weapon.rating },
			{ "day", p.days },
			{ "spd", p.speed },
			{ "luck", p.luck },
			{ "scrolls", p.scrolls },
			{ "level", p.get_level() },
			{ "mana", p.mana },
			{ "notoriety", p.notoriety },
			{ "gold", p.gold },
			{ "silver", p.silver },
			{ "rations", p.rations },
		};

		// Paragraph ID match
		if (std::all_of(val.begin(), val.end(), ::isdigit)) {
			return p.paragraph == atol(val);
		}

		// Match against stat names in the player if they have at least that amount
		// (scrolls, gold, notoriety, exp, skl, day, level etc)
		auto check_score = scorename_map.find(val);
		if (check_score != scorename_map.end() && check_score->second >= amount) return true;
		
		// Flag match
		if (p.has_flag(val)) return true;
		// Inventory item match
		if (p.count_item(val) >= amount) return true;

		return false;
	}

}
