#include <ssod/game_dice.h>
#include <cstdlib> 

int dice() {
	return (rand() % 6) + 1;
}

int d12() {
        return (rand() % 12) + 1;
}

int twodice() {
        return dice() + dice();
}
