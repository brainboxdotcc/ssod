#include <ssod/commands/link.h>
#include <ssod/standalone.h>

dpp::slashcommand link_command::register_command(dpp::cluster& bot) {
	return dpp::slashcommand(std::string(name), "Link your Discord character to the standalone game", bot.me.id);
}

dpp::task<void> link_command::route(const dpp::slashcommand_t& event) {
	try {
		const std::string token = co_await ssod::standalone::link(event.command.usr.id);

		dpp::embed embed;
		embed.set_color(EMBED_COLOUR);

		if (token.empty()) {
			embed.set_title("Account already linked");
			embed.set_description(
			    "Your Discord character is already linked to the standalone game.\n\n"
			    "Use your existing recovery token to access your character. "
			    "Your token is permanent and has not been changed."
			);
		} else {
			embed.set_title("Link your account");
			embed.set_description(
			    "Your standalone recovery token is:\n\n"
			    "```" + token + "```\n"
			    "Open **Account recovery** in the standalone app and enter "
			    "this token in the **Restore an account** box.\n\n"
			    "**Keep this token private.** Anyone with it can access your "
			    "character. It does not expire."
			);
			embed.set_image("https://images.ssod.org/resource/recovery.png");
		}

		event.reply(
		    dpp::message()
			.add_embed(embed)
			.set_flags(dpp::m_ephemeral)
		);
	} catch (const std::exception& e) {
		event.owner->log(
		    dpp::ll_error,
		    "Unable to link standalone account: " + std::string(e.what())
		);

		dpp::embed embed;
		embed.set_color(0xC0392B);
		embed.set_title("Account linking unavailable");
		embed.set_description(
		    "Unable to link your account at the moment. Please try again later."
		);

		event.reply(
		    dpp::message()
			.add_embed(embed)
			.set_flags(dpp::m_ephemeral)
		);
	}

	co_return;
}