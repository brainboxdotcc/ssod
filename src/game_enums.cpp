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
#include <ssod/game_enums.h>

const char* player_race_s[] = {
	"Error",
	"Human",
	"Elf",
	"Orc",
	"Dwarf",
	"Lesser Orc",
	"Barbarian",
	"Goblin",
	"Dark Elf"
};

const char* player_profession_s[] = {
	"Error",
	"Warrior",
	"Wizard",
	"Thief",
	"Woodsman",
	"Assassin",
	"Mercenary"
};


const char* profession(player_profession p) {
	return player_profession_s[p];
}

const char* race(player_race r) {
	return player_race_s[r];
}
