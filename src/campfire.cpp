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
#include <ssod/game.h>
#include <ssod/game_player.h>
#include <ssod/game_util.h>
#include <ssod/component_builder.h>
#include <ssod/emojis.h>
#include <ssod/aes.h>
#include <ssod/database.h>

using namespace i18n;

void campfire(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	std::stringstream content;

	std::vector<dpp::embed_field> fields;

	content << "## " << tr("CAMPFIRE", event) << "\n\n";
	content << "### " << tr("RECIPES", event) << "\n\n";

	auto recipes = db::query("SELECT food.id, food.name, food.description, GROUP_CONCAT(ingredient_name ORDER BY ingredient_name) AS ingredients, stamina_change, skill_change, luck_change, speed_change, value FROM food JOIN ingredients ON food_id = food.id GROUP BY food.id, food.name, food.description ORDER BY value DESC");
	auto ingredients = db::query(
		"SELECT DISTINCT game_owned_items.id, item_desc FROM game_owned_items JOIN ingredients ON item_desc = ingredient_name WHERE user_id = ?"
		" UNION "
		"SELECT DISTINCT game_owned_items.id, item_desc FROM game_owned_items JOIN food ON item_desc = food.name WHERE user_id = ?",
		{event.command.usr.id, event.command.usr.id}
	);
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
				my_ingredients.erase(i);
				checked++;
			}
		}
		if (checked >= recipe_ingredients.size()) {
			can_cook.emplace_back(recipe);
		}
	}

	for (auto& cookable : can_cook) {
		content << "__**" << cookable.at("name") << "**__\n";
		content << tr("INGREDIENTS", event) <<" " << cookable.at("ingredients") << "\n";
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
		content << tr("VALUE", event) << " " << cookable.at("value") << " 🪙";
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
	embed.fields = fields;
	m.embeds = { embed };

	event.reply(event.command.type == dpp::it_application_command ? dpp::ir_channel_message_with_source : dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [event, &bot, m](const auto& cc) {
		if (cc.is_error()) {
			bot.log(dpp::ll_error, "Internal error displaying campfire:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
			event.reply(dpp::message("Internal error displaying campfire:\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```").set_flags(dpp::m_ephemeral));
		}
	});
}

