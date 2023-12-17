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
#include <ssod/ssod.h>
#include <ssod/database.h>
#include <ssod/commands/lore.h>
#include <ssod/database.h>
#include <ssod/sentry.h>
#include <ssod/game_date.h>
#include <ssod/component_builder.h>
#include <filesystem>

namespace fs = std::filesystem;


std::string to_title(std::string s)
{
	bool last{true};
	for (char& c : s) {
		c = last ? std::toupper(c) : std::tolower(c);
		last = std::isspace(c);
	}
	return s;
}

void page(const dpp::interaction_create_t& event, bool document, const std::string& path = "") {
	dpp::cluster* bot = event.from->creator;
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("Encyclopaedia Cryptillius")
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_image("attachment://app_logo.png")
		.set_description("Select a choice from the options below to read information about the game world, its characters and your quest's background.");
	dpp::message m;
	component_builder cb(m);
	if (document) {
		embed.set_description(dpp::utility::read_file(path));
		fs::path fullpath(path);
		fullpath.remove_filename();
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id("lore;" + fullpath.string() + "/")
			.set_label("Back")
			.set_style(dpp::cos_secondary)
			.set_emoji("⬆", 0, false)
		);
		m = cb.get_message();
	} else {
		for (const auto& entry : fs::directory_iterator(path.empty() ? "../resource/lore/" : path)) {
			if (entry.is_directory()) {
				/* Show directories as categories */
				cb.add_component(dpp::component()
					.set_type(dpp::cot_button)
					.set_id("lore;" + entry.path().string())
					.set_label(to_title(replace_string(replace_string(entry.path(), "../resource/lore/", ""), "-", " ")))
					.set_style(dpp::cos_secondary)
					.set_emoji("📂", 0, false)
				);
			} else if (entry.is_regular_file()) {
				/* Show regular files as entries */
				fs::path fullpath(entry.path());
				std::string label = fullpath.filename();
				fullpath.remove_filename();
				cb.add_component(dpp::component()
					.set_type(dpp::cot_button)
					.set_id("lore-read;" + entry.path().string())
					.set_label(replace_string(to_title(replace_string(label, "-", " ")), ".md", ""))
					.set_style(dpp::cos_secondary)
					.set_emoji("📃", 0, false)
				);
			}
		}
		m = cb.get_message();
		m.add_file("app_logo.png", dpp::utility::read_file("../resource/app_logo.png"));
	}
	m.add_embed(embed);
	event.reply(m.set_flags(dpp::m_ephemeral));
}

dpp::slashcommand lore_command::register_command(dpp::cluster& bot) {
	bot.on_button_click([](const dpp::button_click_t& event) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.custom_id, ";");
		if ((parts[0] == "lore" || parts[0] == "lore-read") && parts.size() >= 2) {
			page(event, parts[0] == "lore-read", parts[1]);
		}
	});
	return dpp::slashcommand("lore", "Show lore pages about the game world", bot.me.id);
}

void lore_command::route(const dpp::slashcommand_t &event) {
	page(event, false);
}
