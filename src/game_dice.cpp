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
#include <ssod/game_dice.h>
#include <random>
#include <array>
#include <algorithm>

/* Seed sequence mersainne twister seeding */
std::array<int, 624> seed_data;
std::random_device r;
const int* iv = std::generate_n(seed_data.data(), seed_data.size(), std::ref(r));
std::seed_seq seq(std::begin(seed_data), std::end(seed_data));

std::mt19937 mt(seq);
std::uniform_int_distribution uniform_d6{ 1, 6 };
std::uniform_int_distribution uniform_d12{ 1, 12 };

int d_random(int first, int last) {
	std::uniform_int_distribution uniform{ first, last };
	return uniform(mt);
}

int dice() {
	return uniform_d6(mt);
}

int d12() {
        return uniform_d12(mt);
}

int twodice() {
        return dice() + dice();
}
