#include <ssod/commands/link.h>
#include <ssod/standalone.h>
#include <ssod/ssod.h>

using namespace i18n;

dpp::slashcommand link_command::register_command(dpp::cluster& bot) {
	return dpp::slashcommand(std::string(name), "Link your Discord character to the standalone game", bot.me.id);
}

dpp::task<void> link_command::route(const dpp::slashcommand_t& event) {
	try {
		const std::string token = co_await ssod::standalone::link(event.command.usr.id);

		dpp::embed embed;
		embed.set_color(EMBED_COLOUR);

		if (token.empty()) {
			embed.set_title(tr("LINK_ALREADY_TITLE", event));
			embed.set_description(tr("LINK_ALREADY_DESCRIPTION", event));
		} else {
			embed.set_title(tr("LINK_SUCCESS_TITLE", event));
			embed.set_description(tr("LINK_SUCCESS_DESCRIPTION", event, token));
			embed.set_image("https://images.ssod.org/resource/recovery.png");
		}

		event.reply(
		    dpp::message()
			.add_embed(embed)
			.set_flags(dpp::m_ephemeral)
		);

		if (!token.empty()) {
			try {
				const auto first = co_await event.owner->co_direct_message_create(event.command.usr.id, dpp::message(tr("LINK_DM", event)));
				if (first.is_error()) {
					event.owner->log(dpp::ll_warning, "Unable to deliver link recovery instructions via DM: " + first.get_error().message);
				} else {
					const auto second = co_await event.owner->co_direct_message_create(event.command.usr.id, dpp::message(token));
					if (second.is_error()) {
						event.owner->log(dpp::ll_warning, "Unable to deliver link recovery token via DM: " + second.get_error().message);
					}
				}
			} catch (const std::exception& e) {
				event.owner->log(dpp::ll_warning, "Unable to deliver link recovery token via DM: " + std::string(e.what()));
			}
		}
	} catch (const std::exception& e) {
		event.owner->log(dpp::ll_error, "Unable to link standalone account: " + std::string(e.what()));
		event.reply(
		    dpp::message()
			.add_embed(dpp::embed()
			    .set_color(0xC0392B)
			    .set_title(tr("LINK_ERROR_TITLE", event))
			    .set_description(tr("LINK_ERROR_DESCRIPTION", event))
			)
			.set_flags(dpp::m_ephemeral)
		);
	}

	co_return;
}