/************************************************************************************
 *
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023,2024 Craig Edwards <brain@ssod.org>
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
#include <string>
#include <fmt/format.h>
#include <ssod/ssod.h>
#include <ssod/game_player.h>
#include <ssod/game_util.h>
#include <ssod/component_builder.h>
#include <ssod/emojis.h>
#include <ssod/aes.h>
#include <ssod/database.h>

using namespace i18n;

dpp::task<void> campfire(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	std::stringstream content;

	std::vector<dpp::embed_field> fields;

	content << "## " << tr("CAMPFIRE", event) << "\n\n";

	/* All available recipes - this is cached with 5 minute expiry */
	auto recipes = db::query("SELECT food.id, food.name, food.description, GROUP_CONCAT(ingredient_name ORDER BY ingredient_name) AS ingredients, stamina_change, skill_change, luck_change, speed_change, value FROM food JOIN ingredients ON food_id = food.id GROUP BY food.id, food.name, food.description ORDER BY value DESC", {}, 300.0);
	/* All the user's available ingredients and cooked food items, not grouped */
	auto ingredients = co_await db::co_query(
		"SELECT DISTINCT game_owned_items.id, item_desc FROM game_owned_items JOIN ingredients ON item_desc = ingredient_name WHERE user_id = ?"
		" UNION "
		"SELECT DISTINCT game_owned_items.id, item_desc FROM game_owned_items "
		"JOIN food ON item_desc = food.name WHERE user_id = ?",
		{event.command.usr.id, event.command.usr.id}
	);
	/* Stacked food and ingredient items, with quantities for display */
	auto stacked_ingredients = co_await db::co_query(
		"SELECT item_desc, SUM(qty) AS qty FROM "
		"(SELECT game_owned_items.id, item_desc, COUNT(item_desc) AS qty FROM game_owned_items WHERE user_id = ? AND (SELECT COUNT(*) FROM ingredients WHERE ingredient_name = item_desc LIMIT 1) > 0 "
		"GROUP BY game_owned_items.id, item_desc "
		"UNION "
		"SELECT game_owned_items.id, item_desc, COUNT(item_desc) AS qty FROM game_owned_items JOIN food ON item_desc = food.name WHERE user_id = ? GROUP BY game_owned_items.id, item_desc) "
		"derived GROUP BY item_desc",
		{event.command.usr.id, event.command.usr.id}
	);
	/* Player can cook generic rations */
	auto meat = co_await db::co_query(
		"SELECT DISTINCT game_owned_items.id, item_desc FROM game_owned_items JOIN ingredients ON item_desc = ingredient_name WHERE user_id = ? AND ingredient_name LIKE '%meat%' ORDER BY RAND()",
		{event.command.usr.id}
	);

	/**
	 * Iterate through the player's list of available grouped ingredients and display it.
	 * By "grouped" this means that it is shown in stacks.
	 */
	content << "### " << tr("AVAILABLE_INGREDIENTS", event) << "\n\n";
	for (auto& stack : stacked_ingredients) {
		std::string item{stack.at("item_desc")};
		if (event.command.locale.substr(0, 2) != "en") {
			auto ingredients_q = co_await db::co_query(
				"SELECT ingredient_name, translation FROM ingredients "
				"LEFT JOIN translations ON table_col = 'ingredients/ingredient_name' AND row_id = ingredients.id AND language = ? "
				"where ingredient_name = ?",
				{event.command.locale.substr(0, 2), item}
			);
			if (!ingredients_q.empty() && !ingredients_q[0].at("translation").empty()) {
				item = ingredients_q[0].at("translation");
			}
		}
		content << sprite::rawmeat.get_mention() << " ";
		if (stack.at("qty") != "1") {
			content << stack.at("qty") << "x ";
		}
		content << item << "\n";
	}
	if (stacked_ingredients.empty()) {
		content << tr("NO_INGREDIENTS", event) << "\n";
	}

	content << "\n";

	dpp::component cook_menu;
	cook_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(1)
		.set_placeholder(tr("COOK_RECIPE", event))
		.set_id(security::encrypt("cook"));

	/*
	 * Determine which of the master recipe list is available for the player to cook right now
	 */
	db::resultset can_cook;
	for (auto& recipe : recipes) {
		std::vector<std::string> recipe_ingredients = dpp::utility::tokenize(recipe.at("ingredients"), ",");
		std::vector<std::string> my_ingredients;
		for (auto& ingredient : ingredients) {
			my_ingredients.emplace_back(ingredient.at("item_desc"));
		}
		size_t checked{0};
		for (auto& check_ingredient : recipe_ingredients) {
			auto i = std::find(my_ingredients.begin(), my_ingredients.end(), check_ingredient);
			if (i != my_ingredients.end()) {
				/* This ingredient is required by the recipe, remove an instance of it */
				my_ingredients.erase(i);
				checked++;
			}
		}
		if (checked >= recipe_ingredients.size()) {
			/* All ingredients for this recipe are available */
			can_cook.emplace_back(recipe);
		} else if (dpp::lowercase(recipe.at("name")).find("rations") != std::string::npos && !meat.empty()) {
			can_cook.emplace_back(recipe);
		}
	}

	content << "### " << tr("RECIPES", event) << "\n\n";
	size_t index{0};
	for (auto& cookable : can_cook) {
		std::string name{cookable.at("name")}, description{cookable.at("description")}, ingredients_list{cookable.at("ingredients")};
		if (event.command.locale.substr(0, 2) != "en") {
			/* For non-english game, translate the food names, descriptions and ingredient list */
			auto food_q = co_await db::co_query(
				"SELECT food.id, food.name, "
				"(SELECT translation FROM translations WHERE table_col = 'food/name' AND row_id = food.id AND language = ?) AS translate_name, "
				"(SELECT translation FROM translations WHERE table_col = 'food/description' AND row_id = food.id AND language = ?) AS translate_description "
				"FROM food WHERE name = ? LIMIT 1; ",
				{event.command.locale.substr(0, 2), event.command.locale.substr(0, 2), name}
			);
			if (!food_q.empty()) {
				name = !food_q[0].at("translate_name").empty() ? food_q[0].at("translate_name") : name;
				description = !food_q[0].at("translate_description").empty() ? food_q[0].at("translate_description") : description;
				auto ingredients_q = co_await db::co_query(
					"SELECT ingredient_name, translation FROM ingredients "
					"LEFT JOIN translations ON table_col = 'ingredients/ingredient_name' AND row_id = ingredients.id AND language = ? "
					"where food_id = ?",
					{event.command.locale.substr(0, 2), food_q[0].at("id")}
				);
				ingredients_list.clear();
				for (auto& ingredient : ingredients_q) {
					std::string ing_name{!ingredient.at("translation").empty() ? ingredient.at("translation") : ingredient.at("ingredient_name")};
					ingredients_list.append(ing_name).append(",");
				}
				if (!ingredients_list.empty()) {
					ingredients_list = ingredients_list.substr(0, ingredients_list.length() - 1);
				}
			}
		}
		cook_menu.add_select_option(dpp::select_option(tr("COOK_FOOD", event, name), cookable.at("name") + ";" + std::to_string(++index)).set_emoji(sprite::cooked_meat.name, sprite::cooked_meat.id));
		content << sprite::cooked_meat.get_mention() << " ";
		content << "__**" << name << "**__\n";
		content << "*" << description << "*\n";
		content << "**" << tr("INGREDIENTS", event) << "** " << ingredients_list << "\n";
		if (atol(cookable.at("stamina_change"))) {
			content << tr("STAMINA", event) << " +" << cookable.at("stamina_change") << " ";
		}
		if (atol(cookable.at("skill_change"))) {
			content << tr("SKILL", event) << " +" << cookable.at("skill_change") << " ";
		}
		if (atol(cookable.at("luck_change"))) {
			content << tr("LUCK", event) << " +" << cookable.at("luck_change") << " ";
		}
		if (atol(cookable.at("speed_change"))) {
			content << tr("SPEED", event) << " +" << cookable.at("speed_change") << " ";
		}
		content << tr("VALUE", event) << " " << cookable.at("value") << " 🪙\n\n";
	}
	if (can_cook.empty()) {
		content << tr("NO_RECIPES", event) << "\n";
	}

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{
			.text = tr("CAMPFOOTER", event, p.name),
			.icon_url = bot.me.get_avatar_url(),
			.proxy_url = "",
		})
		.set_thumbnail("https://images.ssod.org/resource/campfire.png")
		.set_colour(EMBED_COLOUR)
		.set_description(content.str());

	dpp::message m;
	component_builder cb(m);

	cb.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id(security::encrypt("exit_campfire"))
		.set_label(tr("BACK", event))
		.set_style(dpp::cos_primary)
		.set_emoji(sprite::magic05.name, sprite::magic05.id)
	);
	cb.add_component(help_button(event));
	m = cb.get_message();

	if (!cook_menu.options.empty()) {
		m.add_component(dpp::component().add_component(cook_menu));
	}
	embed.fields = fields;
	m.embeds = { embed };

	event.reply(event.command.type == dpp::it_application_command ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying campfire:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
			event.reply(dpp::message("Internal error displaying campfire:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```").set_flags(dpp::m_ephemeral));
		}
	});
}

