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
#include <ssod/ssod.h>
#include <ssod/achievement.h>

using namespace i18n;

struct sneaktest_tag : public tag {
	sneaktest_tag() { register_tag<sneaktest_tag>(); }
	static constexpr std::string_view tags[]{"<sneaktest"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		paragraph_content >> p_text;

		extract_to_quote(p_text, paragraph_content, '"');

		std::string monster_name = extract_value(p_text);
		paragraph_content >> p_text;
		long monster_sneak = extract_value_number(p_text);
		output << "\n**" << monster_name << "** *" << tr("SNEAK", current_player.event) << " " << monster_sneak << "*,";
		p.auto_test = current_player.sneak_test(monster_sneak);
		output << (p.auto_test ? " **" + tr("PASS", current_player.event) + "**!\n" : " **" + tr("FAIL", current_player.event) + "**!\n");
		p.words++;
		p.safe = false;
		achievement_check("TEST_SNEAK", current_player.event, current_player, {{"success", p.auto_test}, {"enemy", monster_name}});

		co_return;
	}
};

static sneaktest_tag self_init;
