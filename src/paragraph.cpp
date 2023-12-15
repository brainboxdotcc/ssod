#include <dpp/dpp.h>
#include <ssod/paragraph.h>
#include <ssod/database.h>
#include <ssod/game_player.h>
#include <ssod/game_dice.h>

paragraph::paragraph(uint32_t paragraph_id, player current, dpp::snowflake user_id) {
	auto location = db::query("SELECT * FROM game_locations WHERE id = ?", {paragraph_id});
	if (location.empty()) {
		throw dpp::logic_exception("Invalid location, internal error");
	}
	id = paragraph_id;
	text = location[0].at("data");
	combat_disabled = location[0].at("combat_disabled") == "1";
	magic_disabled = location[0].at("magic_disabled") == "1";
	theft_disabled = location[0].at("theft_disabled") == "1";
	chat_disabled = location[0].at("chat_disabled") == "1";
	auto dropped = db::query("SELECT * FROM game_dropped_items WHERE location_id = ?", {paragraph_id});
	for (const auto& dropped_item : dropped) {
		dropped_items.push_back(item{ .name = dropped_item.at("item_desc"), .flags = dropped_item.at("item_flags") });
	}
	parse(current, user_id);
	// TODO: Nav links
}

// extracts from any NAME=Value pair
std::string extract_without_quotes(const std::string& p_text) {
	std::string item_name;
	bool copying{false};
	for (const char c : p_text) {
		if (c == '=' || c == ' ' || c == '>') {
			copying = !copying;
			continue;
		}
		if (copying) {
			item_name += c;
		}
	}
	return item_name;
}

// extracts a value from any NAME="Value" pair
std::string extract_value(const std::string& p_text) {
	if (p_text.find("\"") == std::string::npos) {
		return extract_without_quotes(p_text);
	}
	std::string item_name;
	bool copying{false};
	for (const char c : p_text) {
		if (c == '"') {
			copying = !copying;
			continue;
		}
		if (copying) {
			item_name += c;
		}
	}
	return item_name;
}

long extract_value_number(const std::string& p_text)
{
	return atol(extract_value(p_text).c_str());
}

bool global_set(const std::string& flag) {
	return db::query("SELECT flag FROM game_global_flags WHERE flag = ?", {flag}).size() > 0;
}

bool comparison(std::string condition, long C1, const std::string& C2, int g_dice) {
	long C = C2 == "dice" ? g_dice : atol(C2.c_str());
	condition = dpp::lowercase(condition);
	if (condition == "eq" && C1 == C) {
		return true;
	} else if (condition == "gt" && C1 > C) {
		return true;
	} else if (condition == "lt" && C1 < C) {
		return true;
	} else if (condition == "ne" && C1 != C) {
		return true;
	} else {
		return false;
	}
}

// returns true if the user hasnt found an item before
bool not_got_yet(uint32_t paragraph, const std::string& item, const std::string& gotfrom) {
	std::string f{" [" + item + std::to_string(paragraph) + "]"};
	return gotfrom.find(f) == std::string::npos;
}

void paragraph::parse(player current_player, dpp::snowflake user_id) {
	std::stringstream paragraph_content(text);
	std::stringstream output;
	std::string p_text, LastLink;
	size_t links{0}, words{0};
	std::string tag;
	bool last_was_link{false};
	bool display{true};
	long after_fragment{0}; // paragraph fragment to start at
	// (each combat increments the current fragment by one)
	long current_fragment{0};	
	bool auto_test{false}, didntmove{false};
	int g_dice{0};

	while (!paragraph_content.eof()) {
		paragraph_content >> p_text;
		std::string neat_version{p_text};

		if (paragraph_content.eof()) {
			break;
		}	

			
		if (dpp::lowercase(p_text) == "<combat") {
			// combat tag
			links++;
			paragraph_content >> p_text;

			while (*p_text.rbegin() != '"') {
				std::string extra;
				paragraph_content >> extra;
				p_text += " " + extra;
			}							
				
			std::string MonsterName = extract_value(p_text);
			paragraph_content >> p_text;
			std::string MonsterSkill = extract_value(p_text);
			paragraph_content >> p_text;
			std::string MonsterStamina = extract_value(p_text);
			paragraph_content >> p_text;
			std::string MonsterArmour = extract_value(p_text);
			paragraph_content >> p_text;
			std::string MonsterWeapon = extract_value(p_text);

			if (current_fragment == after_fragment) {
				// when combat link is finished it goes back to the
				// paragraph it came from, but the next fragment of it.
				// fragments can only be requested on a paragraph
				// that contains at least one combat.
				output << "\n**" << MonsterName << "**\n";
				words++;
				break;
			}
			current_fragment++;
			continue;
		}
			
		if (current_fragment < after_fragment) {
			// nothing should be displayed that comes before the
			// desired fragment!
			continue;
		}

		if (dpp::lowercase(p_text) == "<else>") {
			// simply invert the display flag for anything inside an
			// <else> tag...
			display = !display;
			continue;
		}

		if (dpp::lowercase(p_text) == "<endif>") {
			display = true;
			continue;
		}

		if (dpp::lowercase(p_text) == "<sneaktest") {
			// sneak test tag
			paragraph_content >> p_text;

			while (*p_text.rbegin() != '"') {
				std::string extra;
				paragraph_content >> extra;
				p_text += " " + extra;
			}

			std::string MonsterName = extract_value(p_text);
			paragraph_content >> p_text;
			long MonsterSneak = extract_value_number(p_text);
			output << "\n***" << MonsterName << "** *Sneak " << MonsterSneak << "*,";
			auto_test = current_player.sneak_test(MonsterSneak);
			if (auto_test) {
				output << " **PASSED**!\n";
			} else {
				output << " **FAILED**!\n";
			}
			continue;
		}

		if (dpp::lowercase(p_text) == "<set") {
			// set a state-flag
			paragraph_content >> p_text;
			p_text = dpp::lowercase(p_text.substr(0, p_text.length() - 1));
			current_player.gotfrom += " gamestate_" + p_text;
			current_player.save(user_id);
			continue;
		}

		if (dpp::lowercase(p_text) == "<setglobal") {
			paragraph_content >> p_text;
			p_text = dpp::lowercase(p_text.substr(0, p_text.length() - 1));
			db::query("REPLACE INTO game_global_flags (flag) VALUES(?)", {p_text});
		}

		if (dpp::lowercase(p_text) == "<unsetglobal") {
			paragraph_content >> p_text;
			p_text = dpp::lowercase(p_text.substr(0, p_text.length() - 1));
			db::query("DELETE FROM game_global_flags WHERE flag = ?", {p_text});
		}

		if (dpp::lowercase(p_text) == "<test") {
			// test score tag
			paragraph_content >> p_text;
			p_text = dpp::lowercase(p_text);

			if (p_text.find("luck>") != std::string::npos) {
				output << " Test your __**luck**__. ";
				auto_test = current_player.test_luck();
				current_player.save(user_id);
				continue;
			}

			if (p_text.find("stamina>") != std::string::npos) {
				output << " Test your __**stamina**__. ";
				auto_test = current_player.test_stamina();
				current_player.save(user_id);
				continue;
			}
			if (p_text.find("skill>") != std::string::npos) {
				output << " Test your __**skill**__. ";
				auto_test = current_player.test_skill();
				current_player.save(user_id);
				continue;
			}
			if (p_text.find("speed>") != std::string::npos) {
				output << " Test your __**speed**__. ";
				auto_test = current_player.test_speed();
				current_player.save(user_id);
				continue;
			}
			if (p_text.find("exp>") != std::string::npos) {
				output << " Test your __**experience**__. ";
				auto_test = current_player.test_experience();
				current_player.save(user_id);
				continue;
			}
		}

		if (dpp::lowercase(p_text) == "<time>") {
			if (!didntmove) {
				// time advancement
				//current_player.remove_day();
				current_player.eat_ration();
				current_player.save(user_id);
			}
			continue;
		}

		if (dpp::lowercase(p_text) == "<eat>") {
			if (!didntmove) {
				// just eat a ration, or loose stamina
				current_player.eat_ration();
				current_player.save(user_id);
			}
			continue;
		}

		if (dpp::lowercase(p_text) == "<dice>") {
			g_dice = dice();
			continue;
		}
		if (dpp::lowercase(p_text) == "<d12>") {
			g_dice = d12();
			continue;
		}
		if (dpp::lowercase(p_text) == "<2d6>") {
			g_dice = dice() + dice();
			continue;
		}

		if (dpp::lowercase(p_text) == "<pick") {
			// pick up free items (one-choice)
			paragraph_content >> p_text;

			while (p_text.length() && *p_text.rbegin() != '"') {
				std::string extra;
				paragraph_content >> extra;
				p_text += " " + extra;
			}

			std::string ItemName = extract_value(p_text);
			paragraph_content >> p_text;
			std::string ItemVal = extract_value(p_text);
			
			// TODO: Action Button
			//sprintf(New,"action=pick&guid=%s&item=%s&val=%s",formData[1],AddEscapes(ItemName),ItemVal);
			output << "\n **" << ItemName << "**\n";
		}

		if (dpp::lowercase(p_text) == "<if") {
			std::string condition;
			paragraph_content >> p_text;
			// -------------------------------------------------------
			// <if item multi-word-item-name>
			// -------------------------------------------------------
			if (dpp::lowercase(p_text) == "item") {
				paragraph_content >> p_text;
				while (p_text.length() && *p_text.rbegin() != '>') {
					std::string extra;
					paragraph_content >> extra;
					p_text += " " + extra;
				}
				p_text = p_text.substr(0, p_text.length() - 1);

				std::string flags;
				display = current_player.has_herb(p_text) || current_player.has_spell(p_text) || current_player.has_possession(p_text);
				continue;
			}
			// -------------------------------------------------------
			// <if flag flagname>
			// -------------------------------------------------------
			if (dpp::lowercase(p_text) == "flag") {
				paragraph_content >> p_text;
				p_text = p_text.substr(0, p_text.length() - 1);
				std::string flag = " gamestate_" + p_text;
				display = (current_player.gotfrom.find(flag) != std::string::npos || global_set(p_text));
				continue;
			}
			if (dpp::lowercase(p_text) == "!flag") {
				paragraph_content >> p_text;
				p_text = p_text.substr(0, p_text.length() - 1);
				std::string flag = " gamestate_" + p_text;
				display = (current_player.gotfrom.find(flag) == std::string::npos && !global_set(p_text));
				continue;
			}
			// -------------------------------------------------------
			// <if scorename gt|lt|eq value>
			// -------------------------------------------------------
			std::string scorename = dpp::lowercase(p_text);
			const std::map<std::string, long> scorename_map = {
				{ "exp", current_player.experience },
				{ "dice", g_dice },
				{ "stm", current_player.stamina },
				{ "skl", current_player.skill },
				{ "arm", current_player.armour.rating },
				{ "wpn", current_player.weapon.rating },
				{ "day", current_player.days },
				{ "spd", current_player.speed },
				{ "luck", current_player.luck },
			};
			auto check = scorename_map.find(scorename);
			if (check != scorename_map.end()) {
				paragraph_content >> condition;
				paragraph_content >> p_text;
				display = comparison(condition, check->second, p_text, g_dice);
				continue;
			}

			if (dpp::lowercase(p_text) == "race") {
				// ------------------------------------------------------
				// <if race x>
				// ------------------------------------------------------
				// if false, nothing displayed until an <endif> is reached.
				paragraph_content >> p_text;
				display =
					(dpp::lowercase(p_text) == "human>" && (current_player.race == race_human || current_player.race == race_barbarian))
						||
					(dpp::lowercase(p_text) == "orc>" && (current_player.race == race_orc || current_player.race == race_goblin))
						||
					(dpp::lowercase(p_text) == "elf>" && (current_player.race == race_elf || current_player.race == race_dark_elf))
						||
					(dpp::lowercase(p_text) == "dwarf>" && current_player.race == race_dwarf)
						||
					(dpp::lowercase(p_text) == "lesserorc>" && current_player.race == race_lesser_orc);
				if (display) {
					continue;
				}
			}

			if (dpp::lowercase(p_text) == "prof") {
				// ------------------------------------------------------
				// <if prof x>
				// ------------------------------------------------------
				// if false, nothing displayed until an <endif> is reached.
				paragraph_content >> p_text;
				display = 
					(dpp::lowercase(p_text) == "warrior>" && (current_player.profession == prof_warrior || current_player.profession == prof_mercenary))
						||
					(dpp::lowercase(p_text) == "wizard>" && current_player.profession == prof_wizard)
						||
 					(dpp::lowercase(p_text) == "thief>" && (current_player.profession == prof_thief || current_player.profession == prof_assassin))
						||
					(dpp::lowercase(p_text) == "woodsman>" && current_player.profession == prof_woodsman);
				if (display) {
					continue;
				}
			}
		}

		if (!display) {
			continue;
		}

		if (dpp::lowercase(p_text) == "<br>") {
			output << "\n";
			continue;
		}
		if (dpp::lowercase(p_text) == "<b>" || dpp::lowercase(p_text) == "</b>") {
			output << "**";
			continue;
		}

		if (dpp::lowercase(p_text) == "<input") {
			// <input prompt="prompt" location="loc_id" value="correct_answer">
			// TODO: Modal Dialog!
			links++;
			paragraph_content >> p_text;
			while (p_text.length() && *p_text.rbegin() != '"') {
				std::string extra;
				paragraph_content >> extra;
				p_text += " " + extra;
			}

			std::string Prompt = extract_value(p_text);
			paragraph_content >> p_text;
			std::string Para = extract_value(p_text);
			paragraph_content >> p_text;
			std::string Correct = extract_value(p_text);
			output << "\n\n";
			//output << "<b>" << Prompt << "</b><br><form action='" << me << "'><input type='hidden' name='action' value='riddle'><input type='hidden' name='guid' value='" << formData[1] << "'><input type='hidden' name='keycode' value='" << Key << "'><input type='text' name='q' value='""'><input type='hidden' name='p' value='" << Para << "'><input type='submit' value='Answer'></form>" << CR << CR;
			output << "TODO: Input Dialog\n";
			continue;
		}

		if (dpp::lowercase(p_text) == "<pickup") {
			std::string item_name, item_flags, flags;

			paragraph_content >> p_text;

			if (dpp::lowercase(p_text) == "scroll>") {
				if (not_got_yet(this->id, "SCROLL", current_player.gotfrom)) {
					current_player.scrolls++;
					current_player.gotfrom += " [SCROLL" + std::to_string(this->id) + "]";
				}
				current_player.save(user_id);
				continue;
			}

			if (dpp::lowercase(p_text) == "gold")
			{
				paragraph_content >> p_text;
				p_text = p_text.substr(0, p_text.length() - 1);
				current_player.add_gold(atoi(p_text.c_str()));
				current_player.save(user_id);
				continue;
			}

			if (dpp::lowercase(p_text) == "silver")
			{
				paragraph_content >> p_text;
				p_text = p_text.substr(0, p_text.length() - 1);
				current_player.add_silver(atoi(p_text.c_str()));
				current_player.save(user_id);
				continue;
			}

			item_name = p_text;
			item_flags = "[[none]]";

			while (p_text.length() && *p_text.rbegin() != '>') {
				paragraph_content >> p_text;
				if (p_text.length() && p_text[0] != '[') {
					item_name += " " + p_text;
				} else {
					item_flags = p_text;
					item_flags = item_flags.substr(0, item_flags.length() - 1);
				}
			}
			if (item_name.length() && *item_name.rbegin() == '>') {
				item_name = item_name.substr(0, item_flags.length() - 1);
			}
			// strip the [ and ] from the item flags...
			for (size_t i = 1; i < item_flags.length() - 1; ++i) {
				flags += item_flags[i];
			}

			if (!not_got_yet(this->id, item_name, current_player.gotfrom)) {
				// crafty player trying to get the same item twice! Not good if its unique!
				continue;
			}
			
			current_player.gotfrom += " [" + item_name + std::to_string(this->id) + "]";
			if (flags == "SPELL") {
				current_player.spells.push_back(item{ .name = item_name, .flags = flags });
			} else if (flags == "HERB") {
				current_player.herbs.push_back(item{ .name = item_name, .flags = flags });
			} else {
				current_player.possessions.push_back(item{ .name = item_name, .flags = flags });
			}

			current_player.save(user_id);
			continue;
		}
		
		if (dpp::lowercase(p_text) == "<drop") {
			paragraph_content >> p_text;
			std::string i{p_text};
			while (p_text.length() && *p_text.rbegin() != '>') {
				paragraph_content >> p_text;
				i += " " + p_text;
			}
			i = i.substr(0, i.length() - 1); // remove '>'
			current_player.drop_possession(item{ .name = i, .flags = "" });
			current_player.drop_spell(item{ .name = i, .flags = "" });
			current_player.drop_herb(item{ .name = i, .flags = "" });
			current_player.save(user_id);
			continue;
		}
		
		if (dpp::lowercase(p_text) == "<mod") {
			std::string mod;
			paragraph_content >> p_text;
			paragraph_content >> mod;
			mod = mod.substr(0, mod.length() - 1); // remove '>'
			long modifier = atol(mod.c_str());
			std::string flag = "MOD" + p_text + mod;

			const std::map<std::string, std::pair<std::string, long*>> modifier_list = {
				{"stm", {"stamina", &current_player.stamina}},
				{"skl", {"skill", &current_player.skill}},
				{"luck", {"luck", &current_player.luck}},
				{"exp", {"experience", &current_player.experience}},
				{"arm", {"armour", &current_player.armour.rating}},
				{"wpn", {"weapon", &current_player.weapon.rating}},
				{"spd", {"speed", &current_player.speed}},
			};
			auto m = modifier_list.find(p_text);
			if (m != modifier_list.end()) {
				// No output if the player's been here before
				if (not_got_yet(this->id, flag, current_player.gotfrom)) {
					current_player.gotfrom += " [" + flag + std::to_string(this->id) + "]";
				} else {
					output << "***Make no changes to your " << m->first << "*** ";
					continue;
				}
				current_player.save(user_id);

				output << " ***" << (modifier < 1 ? "Subtract" : "Add") << abs(modifier) << " " << m->second.first << "*** ";
				*(m->second.second) += modifier;

				current_player.save(user_id);
			}
			continue;

		}

		if (dpp::lowercase(p_text) == "<!--") {
			while (!paragraph_content.eof() && p_text != "--!>") {
				paragraph_content >> p_text;
			}
			continue;
		}

		if (dpp::lowercase(p_text) == "<bank>") {
			output << " ***TODO: Implement banking***\n\n";
		}


		if (dpp::lowercase(p_text) == "<i") {
			// purchase item tag
			paragraph_content >> p_text; // always: NAME="ItemName"
			std::string Value{"[none]"}, Cost;
			std::string ItemName = extract_value(p_text);
			paragraph_content >> p_text; // may be: VALUE="Flags" / COST="cost">
			while (p_text.find("=") == std::string::npos) {
				ItemName += " " + p_text;
				paragraph_content >> p_text;
				if (ItemName.length() && *ItemName.rbegin() == '"') {
					ItemName = ItemName.substr(0, ItemName.length() - 1);
				}
			}

			if (p_text.length() && *p_text.rbegin() != '>') {
				// process VALUE token
				Value = extract_value(p_text);
				paragraph_content >> p_text; // read COST token that MUST now follow on
			}

			// process COST token here: COST="cost">
			Cost = extract_value(p_text);
			output << "\n**Buy: " << Value << " " << ItemName << "** (*" << Cost << " gold*)\n";
		} else {
		
			std::string tag = dpp::lowercase(p_text.substr(0, 20));

			if (tag.find("<paylink=") != std::string::npos && !last_was_link) {
				int i{0};
				std::string pnum, cost;
				while (p_text[i++] != '=');
				while (p_text[i] != ',') {
					cost += p_text[i++];
				}
				i++;
				while (p_text[i] != '>') {
					pnum += p_text[i++];
				}
				if (current_player.gold < atol(cost.c_str())) {
					//Image("link_bad.jpg","You dont have enough gold to do this!");
					navigation_links.push_back(nav_link{ .paragraph = atol(pnum.c_str()), .type = nav_type_disabled_link, .cost = 0 });
				}
				else {
					links++;
					output << directions[links];
					navigation_links.push_back(nav_link{ .paragraph = atol(pnum.c_str()), .type = nav_type_paylink, .cost = atol(cost.c_str()) });
				}
				output << " ";
			} else if (tag.find("<link=") != std::string::npos && !last_was_link) {
				int i{0};
				std::string pnum;
				while (p_text[i++] != '=');
				while (p_text[i] != '>') {
					pnum += p_text[i++];
				}
				if (current_player.stamina < 1 || dpp::lowercase(pnum) == "the") {
					output << " (unable to follow this path)";
				} else {
					links++;
					LastLink = pnum;
					output << directions[links];
					navigation_links.push_back(nav_link{ .paragraph = atol(pnum.c_str()), .type = nav_type_link, .cost = 0 });
				}
				output << " ";
			} else if (tag.find("<autolink=") != std::string::npos && !last_was_link) {
				int i{0};
				std::string pnum;
				while (p_text[i++] != '=');
				while (p_text[i] != '>') {
					pnum += p_text[i++];
				}
				/* TODO: What is this 'the' madness? Find out! Ancient fix? */
				if (current_player.stamina < 1 || dpp::lowercase(pnum) == "the") {
					output << " (unable to follow this path)";
				} else {
					if (auto_test) {
						links++;
						LastLink = pnum;
						output << directions[links];
						navigation_links.push_back(nav_link{ .paragraph = atol(pnum.c_str()), .type = nav_type_autolink, .cost = 0 });
					} else {
						navigation_links.push_back(nav_link{ .paragraph = atol(pnum.c_str()), .type = nav_type_disabled_link, .cost = 0 });
					}
				}
				auto_test = !auto_test; // invert the next autolink...
				output << " ";
			}
			else {
				if (neat_version.find(">") == std::string::npos && !neat_version.empty() && neat_version[0] != '<') {
					output << neat_version << " ";
					words++;
				}
			}
		}

	}

	text = output.str();
}