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
