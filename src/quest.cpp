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
#include <ssod/game_util.h>
#include <ssod/database.h>
#include <ssod/aes.h>
#include <ssod/component_builder.h>
#include <gen/emoji.h>
#include <ssod/lang.h>
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
					.image = "quest-started.png"
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
						.image = "quest-completed.png"
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
						.image = "quest-advanced.png"
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
		if (check_score != scorename_map.end() && check_score->second >= amount) {
			return true;
		}
		
		// Flag match
		if (p.has_flag(val)) {
			return true;
		}
		// Inventory item match
		if (p.count_item(val) >= amount) {
			return true;
		}

		return false;
	}

	dpp::task<void> continue_quest_log(const dpp::interaction_create_t& event, player &p) {
		dpp::cluster& bot = *(event.owner);
		std::stringstream content;
		long max_page{0}, active{0}, total_quests{0};
		std::string lang = event.command.locale.substr(0, 2);

		auto quest_count = co_await db::co_query(
			"SELECT COUNT(*) AS total_quests, SUM(IF(status = 'in_progress', 1, 0)) AS active_quests FROM quest_progress JOIN quests ON quest_id = quests.id WHERE user_id = ? AND status IN ('in_progress', 'completed')",
			{event.command.usr.id}
		);
		total_quests = atol(quest_count.at(0).at("total_quests"));
		max_page = ceil(static_cast<double>(total_quests) / 4.0f);
		active = atol(quest_count.at(0).at("active_quests"));

		auto rs = co_await db::co_query(
			"SELECT quests.id, quest_progress.step_index, quests.category, status, title, description FROM quest_progress JOIN quests ON quest_id = quests.id WHERE user_id = ? AND status IN ('in_progress', 'completed') ORDER BY quest_progress.updated_at DESC LIMIT ?, 4",
			{event.command.usr.id, p.quests_page * 4}
		);

		content << "## " << tr("QUEST_LOG", event) << "\n" << tr("QUEST", event, active, total_quests - active) << "\n\n";

		for (const auto& quest : rs) {
			auto quest_steps = co_await db::co_query(
				"SELECT * FROM quest_steps JOIN quest_progress ON quest_steps.quest_id = quest_progress.quest_id AND user_id = ? AND quest_steps.quest_id = ? WHERE step_index = step_order ORDER BY step_order",
				{event.command.usr.id, quest.at("id")}
			);
			std::string description = quest.at("description"), title = quest.at("title");
			if (lang != "en") {
				auto translated = fetch_translations({"quests/description", "quests/title"}, quest.at("id"), lang);
				if (!translated.empty()) {
					description = translated.at(0).at("translation");
					title = translated.at(1).at("translation");
				}
			}
			content << "**" << title << "**";
			bool completed{false}, failed{false};
			if (quest.at("status") == "completed") {
				content << " *" << tr("COMPLETED", event) << "*";
				completed = true;
			} else if (quest.at("status") == "failed") {
				content << " *" << tr("FAILED", event) << "*";
				failed = true;
			}
			content << "\n```ansi\n";
			content << fmt::format(
				fmt::runtime("\033[2;31m[{0}]\033[0m \033[2;36m{1}\033[0m"),
				tr("quest_type_" + quest.at("category"), event),
				description
			);
			content << "\n```\n";
			for (const auto& step : quest_steps) {
				std::string failure{step.at("failure_text")}, success{step.at("success_text")}, progress{step.at("step_text")};
				if (lang != "en") {
					auto translated = fetch_translations({"quest_steps/failure_text", "quest_steps/step_text"}, quest.at("id"), lang);
					if (!translated.empty()) {
						failure = translated.at(0).at("translation");
						progress = translated.at(1).at("translation");
					}
					translated = fetch_translations({"quest_steps/success_text"}, quest.at("id"), lang);
					if (!translated.empty()) success = translated.at(0).at("translation");
				}
				content << "- " << (!completed ? progress : (failed ? failure : success));
				content << " *" << tr(completed ? "COMPLETED" : "IN_PROGRESS", event) << "*" << "\n";
			}
			content << "\n";
		}

		dpp::embed embed = dpp::embed()
			.set_url("https://ssod.org/")
			.set_footer(dpp::embed_footer{
				.text = tr("PAGE_NAV", event, p.quests_page + 1, max_page),
				.icon_url = bot.me.get_avatar_url(),
				.proxy_url = "",
			})
			.set_colour(EMBED_COLOUR)
			.set_description(content.str());

		dpp::message m;
		component_builder cb(m);

		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("exit_quests"))
			.set_label(tr("BACK", event))
			.set_style(dpp::cos_primary)
			.set_emoji(sprite::magic05.name, sprite::magic05.id)
		);
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("quests_page;" + std::to_string(p.quests_page - 1)))
			.set_style(dpp::cos_secondary)
			.set_emoji("◀\uFE0F")
			.set_disabled(p.book_page <= 0)
		);
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id(security::encrypt("quests_page;" + std::to_string(p.quests_page + 1)))
			.set_style(dpp::cos_secondary)
			.set_emoji("▶")
			.set_disabled(p.quests_page >= max_page - 1)
		);
		cb.add_component(help_button(event));
		m = cb.get_message();
		m.embeds = { embed };

		event.reply(event.command.type == dpp::it_application_command ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
			if (cc.is_error()) {
				bot.log(dpp::ll_error, "Internal error displaying quest log:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
			}
		});

		co_return;
	}


	dpp::task<bool> quest_log_nav(const dpp::interaction_create_t& event, player &p, const std::vector<std::string>& parts) {

		dpp::cluster& bot = *(event.owner);

		if (parts[0] == "exit_quests" && parts.size() == 1) {
			p.in_quest_log = false;
			bot.log(dpp::ll_debug, "CLOSE QUEST LOG");
			co_return true;
		} else if (parts[0] == "quests_page" && parts.size() == 2) {
			p.quests_page = atol(parts[1]);
			bot.log(dpp::ll_debug, "QUESTS PAGE " + parts[1]);
			co_return true;
		}

		co_return false;

	}

}
