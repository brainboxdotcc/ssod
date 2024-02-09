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
#include <ssod/parser.h>
#include <ssod/database.h>
#include <ssod/js.h>

struct script_tag : public tag {
	script_tag() { register_tag<script_tag>(); }
	static constexpr std::string_view tags[]{"<script>"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string script, partial;
		while (partial != "</script>") {
			script += " " + partial;
			paragraph_content >> partial;
		}
		js::run(script, p, current_player, {
			{"player", {
				{"stamina", current_player.stamina},
				{"skill", current_player.skill},
				{"sneak", current_player.sneak},
				{"name", current_player.name},
			}}
		});
	}
};

static script_tag self_init;
