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
#include <ssod/database.h>
#include <ssod/commands/bio.h>
#include <ssod/game_util.h>
#include <fmt/format.h>
#include <ssod/neutrino_api.h>
#include <ssod/config.h>

using namespace i18n;

dpp::slashcommand bio_command::register_command(dpp::cluster& bot) {
	return tr(dpp::slashcommand("cmd_bio", "update_bio", bot.me.id)
		.set_interaction_contexts({dpp::itc_guild, dpp::itc_bot_dm, dpp::itc_private_channel})
                .add_option(
			dpp::command_option(dpp::co_sub_command, "opt_picture", "set_bio_picture")
			.add_option(dpp::command_option(dpp::co_attachment, "opt_image", "image_to_upload", true))
		)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "opt_text", "set_custom_bio")
			.add_option(dpp::command_option(dpp::co_string, "cmd_bio", "bio_to_set", true))
		)
	);
}

dpp::task<void> bio_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.owner;

	dpp::command_interaction cmd_data = event.command.get_command_interaction();
	auto subcommand = cmd_data.options[0];

	dpp::embed embed;
	embed.set_url("https://ssod.org/")
		.set_title(tr("CUSTOM_BIO", event))
		.set_footer(dpp::embed_footer{ 
			.text = tr("REQUESTED_BY", event, event.command.usr.format_username()),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR);


	auto rs = co_await db::co_query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { event.command.usr.id });
	if (event.command.entitlements.empty() && rs.empty()) {
		premium_required(event);
	} else  if (subcommand.name == "text") {
		auto param = subcommand.options[0].value;
		std::string text = std::get<std::string>(param);
		neutrino swear_check(event.owner, config::get("neutrino_user"), config::get("neutrino_password"));
		swear_filter_t swear_filter = co_await swear_check.co_contains_bad_word(text);
		if (!swear_filter.clean) {
			bot.log(dpp::ll_warning, "Potty-mouth bio: " + text + " censored for id: " + event.command.usr.id.str());
			text = swear_filter.censored_content;
		}
		co_await db::co_query("INSERT INTO character_bio (user_id, bio) VALUES(?, ?) ON DUPLICATE KEY UPDATE bio = ?", { event.command.usr.id, text, text });
		embed.set_description(tr("CUSTOM_BIO_SET", event) +"\n\n" + text);
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	} else if (subcommand.name == "picture") {
		auto param = subcommand.options[0].value;
            	dpp::snowflake file_id = std::get<dpp::snowflake>(param);
		dpp::attachment att = event.command.get_resolved_attachment(file_id);
		auto data = co_await bot.co_request(att.url, dpp::m_get);
		std::string filename = event.command.usr.id.str() + fs::path(att.filename).extension().c_str();
		std::fstream file;
		dpp::embed e = embed;
		file.open("../uploads/" + filename, std::ios::app | std::ios::binary);
		file.write(data.body.data(), data.body.length());
		file.close();
		co_await db::co_query("INSERT INTO character_bio (user_id, image_name) VALUES(?, ?) ON DUPLICATE KEY UPDATE image_name = ?", { event.command.usr.id, filename, filename });
		e.set_description(tr("CUSTOM_PIC_UPLOADED", event));
		e.set_image("attachment://" + filename);
		event.reply(dpp::message().add_embed(e).add_file(filename, data.body).set_flags(dpp::m_ephemeral));
	}
	co_return;
}
