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

struct test_tag : public tag {
	test_tag() { register_tag<test_tag>(); }
	static constexpr std::string_view tags[]{"<test"};
	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		// test score tag
		paragraph_content >> p_text;
		p_text = dpp::lowercase(p_text);
		p.words++;
		p.safe = false;

		if (p_text.find("luck>") != std::string::npos) {
			output << " " << tr("TESTLUCK", current_player.event) << " ";
			p.auto_test = current_player.test_luck();
			achievement_check("TEST_LUCK", current_player.event, current_player, {{"success", p.auto_test}});
		} else if (p_text.find("stamina>") != std::string::npos) {
			output << " " << tr("TESTSTM", current_player.event) << " ";
			p.auto_test = current_player.test_stamina();
			achievement_check("TEST_STAMINA", current_player.event, current_player, {{"success", p.auto_test}});
		} else if (p_text.find("skill>") != std::string::npos) {
			output << " " << tr("TESTSKL", current_player.event) << " ";
			p.auto_test = current_player.test_skill();
			achievement_check("TEST_SKILL", current_player.event, current_player, {{"success", p.auto_test}});
		} else if (p_text.find("speed>") != std::string::npos) {
			output << " " << tr("TESTSPD", current_player.event) << " ";
			p.auto_test = current_player.test_speed();
			achievement_check("TEST_SPEED", current_player.event, current_player, {{"success", p.auto_test}});
		} else if (p_text.find("exp>") != std::string::npos) {
			output << " " << tr("TESTXP", current_player.event) << " ";
			p.auto_test = current_player.test_experience();
			achievement_check("TEST_XP", current_player.event, current_player, {{"success", p.auto_test}});
		}
		co_return;
	}
};

static test_tag self_init;
