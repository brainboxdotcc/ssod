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
#pragma once

/**
 * @brief An enumeration used to hold a player's race inside a Player class.
 */
enum player_race {
	race_error = 0,
	race_human = 1,
	race_elf = 2,
	race_orc = 3,
	race_dwarf = 4,
	race_lesser_orc = 5,
	race_barbarian = 6,
	race_goblin = 7,
	race_dark_elf = 8,
};

/**
 * @brief An enumeration used to hold a player's profession inside a Player class.
 */
enum player_profession {
	prof_error = 0,
	prof_warrior = 1,
	prof_wizard = 2,
	prof_thief = 3,
	prof_woodsman = 4,
	prof_assassin = 5,
	prof_mercenary = 6,
};

const char* profession(player_profession p);
const char* race(player_race r);
