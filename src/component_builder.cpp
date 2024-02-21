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

void component_builder::add_embed(const dpp::embed& e) {
	message.add_embed(e);
}

void component_builder::add_file(const std::string& filename, const std::string& content) {
	message.add_file(filename, content);
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
