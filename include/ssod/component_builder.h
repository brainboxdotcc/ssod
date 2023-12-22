#pragma once
#include <dpp/dpp.h>
#include <string>
#include <set>

struct component_builder {
	size_t index{0};
	size_t component_parent{0};
	std::set<std::string> ids;
	dpp::message message;
	component_builder(const dpp::message& m);
	void add_component(const dpp::component& c);
	const dpp::message& get_message();
};

