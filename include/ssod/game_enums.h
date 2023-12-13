#pragma once

/**
 * @brief An enumeration used to hold a player's race inside a Player class.
 */
enum player_race {
	race_error = 0,
	race_human,
	race_elf,
	race_orc,
	race_dwarf,
	race_lesser_orc,
	race_barbarian,
	race_goblin,
	race_dark_elf
};

/**
 * @brief An enumeration used to hold a player's profession inside a Player class.
 */
enum player_profession {
	prof_error = 0,
	prof_warrior,
	prof_wizard,
	prof_thief,
	prof_woodsman,
	prof_assassin,
	prof_mercenary
};

const char* profession(player_profession p);
const char* race(player_race r);
