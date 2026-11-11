#pragma once

#include <ssod/ssod.h>
#include <ssod/command.h>

struct link_command : public command {
	static constexpr std::string_view name{"link"};

	static dpp::slashcommand register_command(dpp::cluster& bot);
	static dpp::task<void> route(const dpp::slashcommand_t& event);
};