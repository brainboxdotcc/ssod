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
				{"stance", current_player.stance},
				{"attack", current_player.attack},
				{"in_combat", current_player.in_combat},
				{"in_inventory", current_player.in_inventory},
				{"in_bank", current_player.in_bank},
				{"in_pvp_picker", current_player.in_pvp_picker},
				{"inv_change", current_player.inv_change},
				{"challenged_by", current_player.challenged_by},
				{"after_fragment", current_player.after_fragment},
				{"name", current_player.name},
				{"race", current_player.race},
				{"profession", current_player.profession},
				{"gender", current_player.gender},
				{"stamina", current_player.stamina},
				{"skill", current_player.skill},
				{"luck", current_player.luck},
				{"sneak", current_player.sneak},
				{"speed", current_player.speed},
				{"silver", current_player.silver},
				{"gold", current_player.gold},
				{"rations", current_player.rations},
				{"experience", current_player.experience},
				{"notoriety", current_player.notoriety},
				{"days", current_player.days},
				{"scrolls", current_player.scrolls},
				{"paragraph", current_player.paragraph},
				{"armour_item", current_player.armour.name},
				{"weapon_item", current_player.weapon.name},
				{"armour", current_player.armour.rating},
				{"weapon", current_player.weapon.rating},
				{"gotfrom", current_player.gotfrom},
				{"last_use", current_player.last_use},
				{"last_strike", current_player.last_strike},
				{"pinned", current_player.pinned},
				{"muted", current_player.muted},
				{"mana", current_player.mana},
				{"mana_tick", current_player.mana_tick},
			}},
		{"paragraph", {
				{"id", p.id},
				{"text", p.text},
				{"secure_id", p.secure_id},
				{"combat_disabled", p.combat_disabled},
				{"magic_disabled", p.magic_disabled},
				{"theft_disabled", p.theft_disabled},
				{"chat_disabled", p.chat_disabled},
				{"links", p.links},
				{"words", p.words},
				{"tag", p.tag},
				{"last_was_link", p.last_was_link},
				{"display", p.display},
				{"current_fragment", p.current_fragment},
				{"auto_test", p.auto_test},
				{"didntmove", p.didntmove},
				{"g_dice", p.g_dice},
			}}
		});
	}
};

static script_tag self_init;
