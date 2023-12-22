#include <ssod/component_builder.h>

component_builder::component_builder(const dpp::message &m) : index(0), component_parent(0), message(m) {
	message.add_component(dpp::component());
}

const dpp::message& component_builder::get_message() {
	if (message.components[component_parent].components.empty()) {
		message.components.erase(message.components.end() - 1);
	}
	return message;
}

void component_builder::add_component(const dpp::component& c) {
	if (index >= 25) {
		/* Already at the max of 5x5 buttons */
		return;
	}
	if (ids.find(c.custom_id) != ids.end()) {
		/* Drop duplicate ids */
		return;
	}
	message.components[component_parent].add_component(c);
	ids.insert(c.custom_id);
	index++;
	if (index && (index % 5 == 0)) {
		message.add_component(dpp::component());
		component_parent++;
	}
}
