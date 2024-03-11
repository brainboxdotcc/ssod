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
#include <ssod/ssod.h>
#include <ssod/game_player.h>
#include <fmt/format.h>
#include <ssod/database.h>
#include <ssod/game_dice.h>
#include <ssod/game_util.h>
#include <ssod/aes.h>

/* List of registering players, creating character profiles. These don't get saved to the database until
 * they name their character at the end of the process. The old version of this used to save them temporarily
 * to a table which would need regular cleaning.
 */
player_list registering_players;

/* List of live players, this is a cache of players from the database who have been interacting with the bot.
 */
player_list live_players;

/* Thread safety */
std::mutex reg_list_lock;
std::mutex live_list_lock;

constexpr long MAX_LEVEL = 34;
constexpr long levels[MAX_LEVEL] = {
	-100000000, 0, 10, 20, 40, 80, 160, 250, 500, 550, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 5000, 8000, 10000, 20000,
	40000, 80000, 160000, 240000, 480000, 960000, 1920000, 3840000, 7680000, 15360000, 30720000, 61440000, 122880000,
};


bool player_is_registering(dpp::snowflake user_id) {
	std::lock_guard<std::mutex> l(reg_list_lock);
	return registering_players.find(user_id) != registering_players.end();
}

uint64_t get_active_player_count() {
	std::lock(reg_list_lock, live_list_lock);
	uint64_t count = registering_players.size() + live_players.size();
	reg_list_lock.unlock();
	live_list_lock.unlock();
	return count;
}

bool player_is_live(const dpp::interaction_create_t& event) {
	{
		std::lock_guard<std::mutex> l(live_list_lock);
		auto f = live_players.find(event.command.usr.id);
		if (f != live_players.end()) {
			return true;
		}
	}
	auto rs = db::query("SELECT * FROM game_users WHERE user_id = ?", { event.command.usr.id });
	if (!rs.empty()) {
		/* Load the player into cache */
		player p(event.command.usr.id);
		p.event = event;
		p.state = state_play;
		{
			std::lock_guard<std::mutex> l(live_list_lock);
			live_players[event.command.usr.id] = p;
		}
		return true;
	}
	return false;
}

player get_registering_player(const dpp::interaction_create_t& event) {
	std::lock_guard<std::mutex> l(reg_list_lock);
	auto f = registering_players.find(event.command.usr.id);
	if (f != registering_players.end()) {
		return f->second;
	}
	player p(true);
	p.event = event;
	registering_players[event.command.usr.id] = p;
	return p;
}

void cleanup_idle_live_players() {
	/**
	 * These functions remove players from the list who havent interacted with the bot in 10 mins.
	 * This just ensures they are removed from local cache,they still exist in the database. Also
	 * serves to rehash the unordered maps saving memory.
	 */
	std::lock_guard<std::mutex> l(live_list_lock);
	player_list copy;
	time_t ten_mins_ago = time(nullptr) - 3600;
	for (const std::pair<const dpp::snowflake, player>& pair : live_players) {
		if (pair.second.last_use > ten_mins_ago) {
			copy[pair.first] = pair.second;
		}
	}
	live_players = copy;
}

void cleanup_idle_reg_players() {
	std::lock_guard<std::mutex> l(reg_list_lock);
	player_list copy;
	time_t ten_mins_ago = time(nullptr) - 3600;
	for (const std::pair<const dpp::snowflake, player>& pair : registering_players) {
		if (pair.second.last_use > ten_mins_ago) {
			copy[pair.first] = pair.second;
		}
	}
	registering_players = copy;
}

void update_registering_player(const dpp::interaction_create_t& event, player p) {
	std::lock_guard<std::mutex> l(reg_list_lock);
	registering_players[event.command.usr.id] = p;
}

void update_live_player(const dpp::interaction_create_t& event, player p) {
	std::lock_guard<std::mutex> l(live_list_lock);
	live_players[event.command.usr.id] = p;
}

void move_from_registering_to_live(const dpp::interaction_create_t& event, player p) {
	std::lock(reg_list_lock, live_list_lock);
	auto f = registering_players.find(event.command.usr.id);
	if (f != registering_players.end()) {
		live_players[event.command.usr.id] = p;
		registering_players.erase(f);
	}
	reg_list_lock.unlock();
	live_list_lock.unlock();
}

void delete_live_player(const dpp::interaction_create_t& event) {
	{
		std::lock_guard<std::mutex> l(live_list_lock);
		auto f = live_players.find(event.command.usr.id);
		if (f != live_players.end()) {
			live_players.erase(f);
		}
	}
	db::query("DELETE FROM game_users WHERE user_id = ?", { event.command.usr.id });
	db::query("DELETE FROM game_default_users WHERE user_id = ?", { event.command.usr.id });
	db::query("DELETE FROM game_default_spells WHERE user_id = ?", { event.command.usr.id });
	db::query("DELETE FROM game_bank WHERE owner_id = ?", { event.command.usr.id });
	db::query("DELETE FROM game_owned_items WHERE user_id = ?", { event.command.usr.id });
}

player get_live_player(const dpp::interaction_create_t& event, bool update_event) {
	std::lock_guard<std::mutex> l(live_list_lock);
	auto f = live_players.find(event.command.usr.id);
	if (f != live_players.end()) {
		return f->second;
	}
	/* Retrieve from database */
	player p(event.command.usr.id);
	if (update_event) {
		p.event = event;
	}
	p.state = state_play;
	live_players[event.command.usr.id] = p;
	return p;
}

player::~player() = default;

long player::get_level() const {
	long level = 1;
	while ((experience >= levels[level]) && (level != MAX_LEVEL)) level++;
	return std::max(1l, level - 1);
}

long player::xp_worth() {
	long level = get_level();
	if (level >= MAX_LEVEL - 1) {
		return ((levels[MAX_LEVEL - 1] - levels[MAX_LEVEL - 2]) / 10);
	}
	return ((levels[level + 1] - levels[level]) / 10);
}

void player::death_xp_loss() {
	long level = get_level();
	if (level > 1) {
		experience -= ((levels[level + 1] - levels[level]) / 4);
		if (experience < levels[level]) {
			experience = levels[level];
		}
	}
}

double player::get_percent_of_current_level() {
	long level = get_level();
	if (level >= MAX_LEVEL - 1) {
		return 0;
	}
	long next_level = levels[level + 1];
	long total_span = next_level - levels[level];
	long through_span = experience - levels[level];
	if (through_span == 0) {
		return 0;
	} else {
		return ((double)through_span / (double)total_span) * 100.0;
	}
}

dpp::message player::get_registration_message(dpp::cluster& cluster, const dpp::interaction_create_t &event) {

	std::string file = matrix_image(race, profession, gender == "male");
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("New Character Creation")
		.set_footer(dpp::embed_footer{ 
			.text = "New player creation for " + event.command.usr.format_username(), 
			.icon_url = cluster.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(fmt::format("Welcome to the world of **Utopia**, {}!\n\n\
Your character is shown below. If you are not happy with your base stats, click **Re-Roll** for new ones.\n\
### Once you commit to this character, you cannot change these base stats without __restarting__.", event.command.usr.get_mention()))
		.add_field("Stamina", std::to_string(stamina) + bonuses(1, race, profession), true)
		.add_field("Skill", std::to_string(skill) + bonuses(2, race, profession), true)
		.add_field("Luck", std::to_string(luck) + bonuses(3, race, profession), true)
		.add_field("Sneak", std::to_string(sneak) + bonuses(4, race, profession), true)
		.add_field("Speed", std::to_string(speed) + bonuses(5, race, profession), true)
		.add_field("Gold", std::to_string(gold), true)
		.add_field("Silver", std::to_string(silver), true)
		.add_field("Rations", std::to_string(rations), true)
		.add_field("Notoriety", std::to_string(notoriety), true)
		.add_field("Armour", fmt::format("{} (Rating {})", armour.name, armour.rating), true)
		.add_field("Weapon", fmt::format("{} (Rating {})", weapon.name, weapon.rating), true)
		.set_image(file);

		dpp::component race_select_menu, profession_select_menu, gender_select_menu;
		race_select_menu.set_type(dpp::cot_selectmenu)
			.set_min_values(1)
			.set_max_values(1)
			.set_required(true)
			.set_placeholder("Select Your Race")
			.set_id(security::encrypt("select_player_race"))
			.add_select_option(dpp::select_option("Human", "1", "The jack-of-all-trades").set_default(race == race_human))
			.add_select_option(dpp::select_option("Elf", "2", "Experts in the arcane and the forests").set_default(race == race_elf))
			.add_select_option(dpp::select_option("Orc", "3", "Battle hardened and never backs down from a fight").set_default(race == race_orc))
			.add_select_option(dpp::select_option("Dwarf", "4", "Small and hardy with a love of treasure").set_default(race == race_dwarf))
			.add_select_option(dpp::select_option("Lesser Orc", "5", "A rebel from the servant-race of the Orcs").set_default(race == race_lesser_orc))
			.add_select_option(dpp::select_option("Barbarian", "6", "A tough human of the northern wastes").set_default(race == race_barbarian))
			.add_select_option(dpp::select_option("Goblin", "7", "A dumber, but craftier creature of Orc blood").set_default(race == race_goblin))
			.add_select_option(dpp::select_option("Dark Elf", "8", "Experts in dark magic and subterfuge").set_default(race == race_dark_elf));
		profession_select_menu.set_type(dpp::cot_selectmenu)
			.set_min_values(1)
			.set_max_values(1)
			.set_placeholder("Select Your Profession")
			.set_required(true)
			.set_id(security::encrypt("select_player_profession"))
			.add_select_option(dpp::select_option("Warrior", "1", "Experts with bladed and blunt weapons").set_default(profession == prof_warrior))
			.add_select_option(dpp::select_option("Wizard", "2", "Skilled in use of magic and potions").set_default(profession == prof_wizard))
			.add_select_option(dpp::select_option("Thief", "3", "Sleight of hand is their game").set_default(profession == prof_thief))
			.add_select_option(dpp::select_option("Woodsman", "4", "Experts in bows, and navigating forests").set_default(profession == prof_woodsman))
			.add_select_option(dpp::select_option("Assassin", "5", "A stealthy and efficient professional killer for hire").set_default(profession == prof_assassin))
			.add_select_option(dpp::select_option("Mercenary", "6", "A sword for hire, who gets the job done... for a price.").set_default(profession == prof_mercenary));
		gender_select_menu.set_type(dpp::cot_selectmenu)
			.set_min_values(1)
			.set_max_values(1)
			.set_placeholder("Select Your Gender")
			.set_required(true)
			.set_default_value("male")
			.set_id(security::encrypt("select_player_gender"))
			.add_select_option(dpp::select_option("Male", "male").set_default(gender == "male"))
			.add_select_option(dpp::select_option("Female", "female").set_default(gender == "female"));

	return dpp::message()
		.add_embed(embed)
		.add_component(dpp::component()
			.add_component(race_select_menu)
		)
		.add_component(dpp::component()
			.add_component(profession_select_menu)
		)
		.add_component(dpp::component()
			.add_component(gender_select_menu)
		)
		.add_component(
			dpp::component()
			.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_reroll"))
				.set_label("Re-Roll")
				.set_style(dpp::cos_danger)
				.set_emoji("🎲")
			)
			.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_herb_spell_selection"))
				.set_label("Continue")
				.set_style(dpp::cos_success)
				.set_emoji("➡️")
			)
			.add_component(help_button())
		).set_flags(dpp::m_ephemeral);
}

bool player::has_herb(std::string herb_name) {
	herb_name = dpp::lowercase(herb_name);
	for (const item& herb : herbs) {
		if (dpp::lowercase(herb.name) == herb_name) {
			return true;
		}
	}
	return false;
}

bool player::has_spell(std::string spell_name) {
	spell_name = dpp::lowercase(spell_name);
	for (const item& spell : spells) {
		if (dpp::lowercase(spell.name) == spell_name) {
			return true;
		}
	}
	return false;
}

bool player::has_possession(std::string name) {
	name = dpp::lowercase(name);
	for (const item& inv : possessions) {
		if (dpp::lowercase(inv.name) == name) {
			return true;
		}
	}
	return false;
}

bool player::drop_possession(const item& i) {
	for (auto inv = possessions.begin(); inv != possessions.end(); ++inv) {
		if (dpp::lowercase(inv->name) == dpp::lowercase(i.name)) {
			possessions.erase(inv);
			inv_change = true;
			return true;
		}
	}
	return false;
}

bool player::drop_spell(const item& i) {
	for (auto inv = spells.begin(); inv != spells.end(); ++inv) {
		if (dpp::lowercase(inv->name) == dpp::lowercase(i.name)) {
			spells.erase(inv);
			inv_change = true;
			return true;
		}
	}
	return false;
}

bool player::drop_herb(const item& i) {
	for (auto inv = herbs.begin(); inv != herbs.end(); ++inv) {
		if (dpp::lowercase(inv->name) == dpp::lowercase(i.name)) {
			herbs.erase(inv);
			inv_change = true;
			return true;
		}
	}
	return false;
}

bool player::has_component_herb(const std::string& spell) {
	std::string s = dpp::lowercase(spell);
	if (s == "fire" && has_herb("fireseeds")) {
		return true;
	}
	if (s == "water" && has_herb("hartleaf")) {
		return true;
	}
	if (s == "light" && has_herb("fireseeds")) {
		return true;
	}
	if (s == "fly" && has_herb("elfbane")) {
		return true;
	}
	if (s == "strength") {
		return true;
	}
	if (s == "x-ray" || s == "xray") {
		return true;
	}
	if (s == "bolt" && has_herb("spikegrass")) {
		return true;
	}
	if (s == "fasthands" && has_herb("orcweed")) {
		return true;
	}
	if (s == "thunderbolt" && has_herb("wizardsivy")) {
		return true;
	}
	if (s == "steal" && has_herb("wizardsivy")) {
		return true;
	}
	if (s == "shield" && has_herb("fireseeds")) {
		return true;
	}
	if (s == "jump" && has_herb("hartleaf")) {
		return true;
	}
	if (s == "open") {
		return true;
	}
	if (s == "spot") {
		return true;
	}
	if (s == "sneak" && has_herb("stickwart")) {
		return true;
	}
	if (s == "esp" && has_herb("stickwart")) {
		return true;
	}
	if (s == "run" && has_herb("elfbane")) {
		return true;
	}
	if (s == "invisible") {
		return true;
	}
	if (s == "shrink" && has_herb("woodweed")) {
		return true;
	}
	if (s == "grow" && has_herb("woodweed")) {
		return true;
	}
	if (s == "air" && has_herb("monkgrass")) {
		return true;
	}
	if (s == "animalcommunication" && has_herb("monkgrass")) {
		return true;
	}
	if (s == "weaponskill" && has_herb("hartleaf")) {
		return true;
	}
	if (s == "healing" && has_herb("wizardsivy")) {
		return true;
	}
	if (s == "woodsmanship" && has_herb("wizardsivy")) {
		return true;
	}
	if (s == "nightvision") {
		return true;
	}
	if (s == "heateyes" && has_herb("fireseeds")) {
		return true;
	}
	if (s == "decipher" && has_herb("blidvines")) {
		return true;
	}
	if (s == "detect" && has_herb("blidvines")) {
		return true;
	}
	if (s == "tracking" && has_herb("blidvines")) {
		return true;
	}
	if (s == "espsurge" && has_herb("hallucinogen")) {
		return true;
	}
	if (s == "afterimage" && has_herb("hallucinogen")) {
		return true;
	}
	if (s == "psychism" && has_herb("hallucinogen")) {
		return true;
	}
	if (s == "spiritwalk" && has_herb("hallucinogen")) {
		return true;
	}
	if (s == "growweapon" && has_herb("woodweed")) {
		return true;
	}
	return false;
}


dpp::message player::get_magic_selection_message(dpp::cluster& cluster, const dpp::interaction_create_t &event) {
	size_t max_spells = (profession == prof_wizard ? 5 : 2);
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title("Magic Selection")
		.set_footer(dpp::embed_footer{ 
			.text = "New player creation for " + event.command.usr.format_username(), 
			.icon_url = cluster.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(fmt::format("Now, select your herbs and magic. Certain spells require herbs in your inventory to function.\n\n\
Read the descriptions of the herbs to determine which types of spells they might provide access to.\n\
### You have selected __{}__ of up to __{}__ herbs, and __{}__ of up to __{}__ spells.\n\n\
Once you are happy with your choices, click **Continue** to name your character.", herbs.size(), 3, spells.size(), max_spells));

	dpp::component herb_select_menu, spell_select_menu;
	herb_select_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(3)
		.set_placeholder("Select Herbs")
		.set_id(security::encrypt("select_player_herbs"))
		.add_select_option(dpp::select_option("Hartleaf", "hartleaf", "A herb associated with movement and water").set_default(has_herb("hartleaf")))
		.add_select_option(dpp::select_option("Elfbane", "elfbane", "Commonly found around fast moving birds").set_default(has_herb("elfbane")))
		.add_select_option(dpp::select_option("Monkgrass", "monkgrass", "Opens the mind to other languages").set_default(has_herb("monkgrass")))
		.add_select_option(dpp::select_option("Fireseeds", "fireseeds", "Used for spells involving heat and fire").set_default(has_herb("fireseeds")))
		.add_select_option(dpp::select_option("Woodweed", "woodweed", "Has properties that allow rapid change in size").set_default(has_herb("woodweed")))
		.add_select_option(dpp::select_option("Blidvines", "blidvines", "Used to reveal what is hidden").set_default(has_herb("blidvines")))
		.add_select_option(dpp::select_option("Stickwart", "stickwart", "Said to allow the user to read minds").set_default(has_herb("stickwart")))
		.add_select_option(dpp::select_option("Spikegrass", "spikegrass", "A very sharp bladed plant").set_default(has_herb("spikegrass")))
		.add_select_option(dpp::select_option("Hallucinogen", "hallucinogen", "Alters perception for the user and those nearby").set_default(has_herb("hallucinogen")))
		.add_select_option(dpp::select_option("Wizards Ivy", "wizardsivy", "A truly powerful but mysterious plant").set_default(has_herb("wizardsivy")))
		.add_select_option(dpp::select_option("Orcweed", "orcweed", "Literally a weed, abundant around Orc camps").set_default(has_herb("orcweed")));
	spell_select_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_placeholder("Select Spells")
		.set_id(security::encrypt("select_player_spells"));
	/* Fill spell select menu only with spells applicable to the chosen herbs up to a max of 25 choices */
	const std::vector<dpp::select_option> all_spells{
		dpp::select_option("Fire", "fire", "Summons fire"),
		dpp::select_option("Water", "water", "Summons Water"),
		dpp::select_option("Light", "light", "Summons light around the caster"),
		dpp::select_option("Fly", "fly", "Allows the caster to fly for a short amount of time"),
		dpp::select_option("Strength", "strength", "Gives the caster increased strength"),
		dpp::select_option("X-Ray", "x-ray", "Allows the caster to see through many objects"),
		dpp::select_option("Bolt", "bolt", "Fires a sharp and hard projectile from the hand of the caster"),
		dpp::select_option("Fast Hands", "fasthands", "Allows for faster movement"),
		dpp::select_option("Thunderbolt", "thunderbolt", "Summons lightning from the heavens!"),
		dpp::select_option("Steal", "steal", "Makes it easier for the caster to steal things"),
		dpp::select_option("Shield", "shield", "Protects the caster from physical attacks"),
		dpp::select_option("Jump", "jump", "Allows the caster to jump superhuman heights"),
		dpp::select_option("Open", "open", "Opens locks that would otherwise remain stubbornly closed"),
		dpp::select_option("Spot", "spot", "Identifies things that are hidden"),
		dpp::select_option("Sneak", "sneak", "Makes the caster stealthy and hard to detect"),
		dpp::select_option("E.S.P.", "esp", "Allows the caster to read minds"),
		dpp::select_option("Run", "run", "Makes the caster supenaturally fast-footed"),
		dpp::select_option("Invisible", "invisible", "Hides the caster from most creatures"),
		dpp::select_option("Shrink", "shrink", "Makes the caster very small, hard to hit and quite vulnerable"),
		dpp::select_option("Grow", "grow", "Makes the caster temporarily very large"),
		dpp::select_option("Air", "air", "Projects a bubble of breathable air around the caster"),
		dpp::select_option("Animal Communication", "animalcommunication", "Talk to animals. Does what it says on the tin."),
		dpp::select_option("Weapon Skill", "weaponskill", "A temporary boost in weapons proficiency"),
		dpp::select_option("Healing", "healing", "Heals the sick and knits closed wounds"),
		dpp::select_option("Woodsmanship", "woodsmanship", "Gives an advantage in forests and jungles"),
		dpp::select_option("Night Vision", "nightvision", "Allows the caster to see in the dark"),
		dpp::select_option("Heat Eyes", "heateyes", "Project blazing heat from the caster's eyeballs"),
		dpp::select_option("Decipher", "decipher", "Read what others cannot"),
		dpp::select_option("Detect", "detect", "Find many things that are hiding from you"),
		dpp::select_option("Tracking", "tracking", "Follow those who do not want to be followed"),
		dpp::select_option("E.S.P. Surge", "espsurge", "A psychic blast that can destroy minds and sanity"),
		dpp::select_option("After Image", "afterimage", "Project ghost-like copies of yourself behind you as you move"),
		dpp::select_option("Psychism", "psychism", "Allows a glimpse into the realm beyond"),
		dpp::select_option("Spirit Walk", "spiritwalk", "Leave your body to see places physically far away"),
		dpp::select_option("Grow Weapon", "growweapon", "Make your weapons bigger. Good luck using them!"),
	};
	
	size_t spell_count = 0;
	for (auto spell : all_spells) {
		if (has_component_herb(spell.value)) {
			spell_select_menu.add_select_option(spell.set_default(has_spell(spell.value)));
			spell_count++;
		}
	}
	if (!spell_count) {
		spell_select_menu.add_select_option(dpp::select_option("Select at least one herb", "0", "You must select some herbs. The herbs facilitate spell casting."));
		spell_select_menu.set_max_values(0);
	} else {
		if (spell_count < max_spells) {
			spell_select_menu.set_max_values(spell_count);
		} else {
			spell_select_menu.set_max_values(max_spells);
		}
	}

	return dpp::message()
		.add_embed(embed)
		.add_component(dpp::component()
			.add_component(herb_select_menu)
		)
		.add_component(dpp::component()
			.add_component(spell_select_menu)
		)
		.add_component(
			dpp::component()
			.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_name"))
				.set_label("Continue")
				.set_style(dpp::cos_success)
				.set_emoji("➡️")
			)
			.add_component(help_button())
		).set_flags(dpp::m_ephemeral);
}

player::player(bool reroll) :
	state(state_roll_stats), in_combat(false), in_inventory(false), in_bank(false), in_pvp_picker(false), inv_change(false),
	challenged_by(0), after_fragment(0), race(player_race::race_error), profession(player_profession::prof_error), stamina(0),
	skill(0), luck(0), sneak(0), speed(0), silver(0), gold(0), rations(0), experience(0), notoriety(0), days(0), scrolls(0),
	last_use(0), last_strike(0), pinned(0), muted(0), mana(0), mana_tick(0) {
	if (reroll) {
		skill = dice() + 5;
		stamina = dice() + 5;
		speed = dice() + 5;
		sneak = dice();
		rations = dice() + 4;
		luck = dice() + dice() + 4;
		gold = 10;
		days = 14;
		notoriety = 0;
		mana = max_mana();
		gender = "male";
		int d;
		while ((d = dice()) == 6);
		race = (player_race)d;
		profession = (player_profession)dice();
		last_resurrect = 0;

		weapon = { .name = "Hunting Dagger", .rating = 1 };
		armour = { .name = "Leather Coat", .rating = 1 };
		possessions.emplace_back(item{ .name = "Hunting Dagger", .flags = "W1" });
		possessions.emplace_back(item{ .name = "Leather Coat", .flags = "A1" });
		possessions.emplace_back(item{ .name = "Stamina Potion", .flags = "ST+4" });
		possessions.emplace_back(item{ .name = "Skill Potion", .flags = "SK+4" });
		breadcrumb_trail = {};
		inv_change = true;
		reset_to_spawn_point();
		gotfrom = "__ITEMS_FROM__";

		if (profession == prof_warrior) {
			add_stamina(5);
		}
		if (race == race_barbarian || race == race_orc) {
			add_stamina(3); 
		}
	}
}

player::player(dpp::snowflake user_id, bool get_backup) : player() {
	db::resultset a_row = db::query(fmt::format("SELECT * FROM {} WHERE user_id = ?", get_backup ? "game_default_users" : "game_users"), {user_id});
	if (a_row.size()) {
		race = (player_race)atoi(a_row[0].at("race"));
		profession = (player_profession)atoi(a_row[0].at("profession"));
		name = a_row[0].at("name");
		stamina = atol(a_row[0].at("stamina"));
		skill = atol(a_row[0].at("skill"));
		luck = atol(a_row[0].at("luck"));
		sneak = atol(a_row[0].at("sneak"));
		speed = atol(a_row[0].at("speed"));
		silver = atol(a_row[0].at("silver"));
		gold = atol(a_row[0].at("gold"));
		rations = atol(a_row[0].at("rations"));
		experience = atol(a_row[0].at("experience"));
		notoriety = atol(a_row[0].at("notoriety"));
		days = atol(a_row[0].at("days"));
		scrolls = atol(a_row[0].at("scrolls"));
		paragraph = atol(a_row[0].at("paragraph"));
		gotfrom = a_row[0].at("gotfrom");
		armour.name = a_row[0].at("armour");
		weapon.name = a_row[0].at("weapon");
		armour.rating = atol(a_row[0].at("armour_rating"));
		weapon.rating = atol(a_row[0].at("weapon_rating"));
		gender = a_row[0].at("gender");
		// TODO: NOTES
		last_use = atoll(a_row[0].at("lastuse"));
		last_strike = atoll(a_row[0].at("laststrike"));
		pinned = atoll(a_row[0].at("pinned"));
		muted = atoll(a_row[0].at("muted"));
		mana = atol(a_row[0].at("mana"));
		mana_tick = atoll(a_row[0].at("manatick"));
		last_resurrect = atoll(a_row[0].at("last_resurrect"));
		after_fragment = 0;
		in_combat = false;
		try {
			json crumbs = json::parse(a_row[0].at("breadcrumb_trail"));
			for (const auto& crumb : crumbs) {
				long p{crumb.get<long>()};
				breadcrumb_trail.push_back(p);
			}
		}
		catch (const std::exception&) {
		}
	}

	auto res = db::query("SELECT item_desc, item_flags FROM game_owned_items WHERE user_id = ?", {user_id});
	for (const auto& a_row : res) {
		const std::string& item_desc = a_row.at("item_desc");
		const std::string& item_flags = a_row.at("item_flags");
		if (item_flags == "SPELL" && !item_desc.empty()) {
			spells.emplace_back(item{ .name = item_desc, .flags = item_flags });
		} else if (item_flags == "HERB" && !item_desc.empty()) {
			herbs.emplace_back(item{ .name = item_desc, .flags = item_flags });
		} else if (!item_desc.empty()) {
			possessions.emplace_back(item{ .name = item_desc, .flags = item_flags });
		}
	}
}

long player::max_mana() {
	if (profession == prof_wizard) {
		return 10 + (get_level() * 6);
	} else {
		return 10 + (get_level() * 2);
	}
}

void player::tick_mana() {
	if (mana_tick < time(nullptr) - 60) {
		// Wizards regain 2 mana per 1 min. other professions gain 1 mana point per min.
		mana_tick = time(NULL);
		mana += (profession == prof_wizard ? 2 : 1);
	}
	mana = std::min(max_mana(), mana);
}

void player::drop_everything() {
	/* Drop everything to floor */
	for (const auto& i : possessions) {
		/* Scrolls arent dropped to the floor */
		if (dpp::lowercase(i.name) != "scroll") {
			db::query("INSERT INTO game_dropped_items (location_id, item_desc, item_flags) VALUES(?,?,?)", {paragraph, i.name, i.flags});
		}
	}
	possessions.clear();
	possessions.emplace_back(item{ .name = "Hunting Dagger", .flags = "W1" });
	possessions.emplace_back(item{ .name = "Leather Coat", .flags = "A1" });
	possessions.emplace_back(item{ .name = "Stamina Potion", .flags = "ST+4" });
	possessions.emplace_back(item{ .name = "Skill Potion", .flags = "SK+4" });
	herbs.clear();
	spells.clear();
	auto rs = db::query("SELECT * FROM game_default_spells WHERE user_id = ?", {event.command.usr.id});
	for (const auto & row : rs) {
		if (row.at("flags") == "HERB") {
			herbs.emplace_back(item{ .name = row.at("name"), .flags = row.at("flags") });
		} else {
			spells.emplace_back(item{ .name = row.at("name"), .flags = row.at("flags") });
		}
	}
	inv_change = true;
}

bool player::save(dpp::snowflake user_id, bool put_backup)
{
	tick_mana();
	db::transaction();

	if (inv_change) {
		db::query("DELETE FROM game_owned_items WHERE user_id = ?", {user_id});
		
		for (const item& posession : possessions) {
			if (posession.name != "[none]") {
				db::query("INSERT INTO game_owned_items (user_id, item_desc, item_flags) VALUES(?,?,?)", {user_id, posession.name, posession.flags});
			}
		}
		for (const item& herb : herbs) {
			if (herb.name != "[none]") {
				db::query("INSERT INTO game_owned_items (user_id, item_desc, item_flags) VALUES(?,?,?)", {user_id, herb.name, herb.flags});
			}
		}
		for (const item& spell : spells) {
			if (spell.name != "[none]") {
				db::query("INSERT INTO game_owned_items (user_id, item_desc, item_flags) VALUES(?,?,?)", {user_id, spell.name, spell.flags});
			}
		}

		inv_change = false;
	}
	last_use = time(nullptr);

	json crumbs = json::array();;
	if (!put_backup) {
		for (long p : breadcrumb_trail) {
			crumbs.push_back(p);
		}
	}

	db::query(fmt::format("INSERT INTO {} (user_id, name, race, profession, stamina, skill, luck, sneak, speed, silver, gold, rations, experience, notoriety, days, scrolls, paragraph, \
	 armour, weapon, gotfrom, armour_rating, weapon_rating, lastuse, laststrike, pinned, muted, mana, manatick, gender, breadcrumb_trail, last_resurrect) \
	 VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) \
	 ON DUPLICATE KEY UPDATE name = ?, race = ?, profession = ?, stamina = ?, skill = ?, luck = ?, sneak = ?, speed = ?, silver = ?, gold = ?, rations = ?, experience = ?, notoriety = ?, days = ?, scrolls = ?, paragraph = ?, \
	 armour = ?, weapon = ?, gotfrom = ?, armour_rating = ?, weapon_rating = ?, lastuse = ?, laststrike = ?, pinned = ?, muted = ?, mana = ?, manatick = ?, gender = ?, breadcrumb_trail = ?, last_resurrect = ?", put_backup ? "game_default_users" : "game_users"),
		{user_id, name, race, profession, stamina, skill, luck, sneak, speed, silver, gold, rations, experience, notoriety, days, scrolls, paragraph,
		armour.name, weapon.name, gotfrom, armour.rating, weapon.rating, last_use, last_strike, pinned, muted, mana, mana_tick, gender, crumbs.dump(), last_resurrect,
		name, race, profession, stamina, skill, luck, sneak, speed, silver, gold, rations, experience, notoriety, days, scrolls, paragraph,
		armour.name, weapon.name, gotfrom, armour.rating, weapon.rating, last_use, last_strike, pinned, muted, mana, mana_tick, gender, crumbs.dump(), last_resurrect}
	);

	db::commit();

	if (put_backup) {
		save(user_id, false);
	}

	return true;
}

void player::add_flag(const std::string flag, long paragraph) {
	gotfrom += " [" + flag + std::to_string(paragraph) + "]";
}

bool player::has_flag(const std::string flag, long paragraph) {
	std::string f{" [" + flag+ std::to_string(paragraph) + "]"};
	return gotfrom.find(f) != std::string::npos;
}

void player::strike() {
	// store this time as the time a hit was last made in combat
	last_strike = time(nullptr);
}

void player::reset_to_spawn_point() {
	switch (race) {
		case race_human:
			paragraph = 1306;
		break;
		case race_orc:
			paragraph = 1306;
		break;
		case race_lesser_orc:
			paragraph = 1325;
		break;
		case race_elf:
			paragraph = 1325;
		break;
		default:
		case race_dwarf:
			paragraph = 1332;
		break;
	}
}

bool player::convert_rations(const item& i) {
	std::string name{dpp::lowercase(i.name)};
	if (name.find("12 ration") != std::string::npos) {
		add_rations(12);
		return true;
	} else if (name.find("5 ration") != std::string::npos) {
		add_rations(5);
		return true;
	} else if (name.find("4 ration") != std::string::npos) {
		add_rations(4);
		return true;
	} else if (name.find("3 ration") != std::string::npos) {
		add_rations(3);
		return true;
	} else if (name.find("2 ration") != std::string::npos) {
		add_rations(2);
		return true;
	} else if (name.find("ration") != std::string::npos) {
		add_rations(1);
		return true;
	}
	return false;
}

std::string player::get_flags() {
	return gotfrom;
}

bool player::sneak_test(long monster_sneak) {
	return ((sneak + dice()) > (monster_sneak + dice()));
}

void player::add_stamina(long modifier) {
	if (stamina < 1) {
		// cant add stamina to a dead person (stops resurrect bugs)
		return;
	}
	stamina = std::max((long)0, stamina + modifier);
	stamina = std::min(stamina, max_stamina());
}

dpp::snowflake player::get_user() {
	return event.command.usr.id;
}

void player::add_experience(long modifier) {
	long old_value = get_level();
	experience = std::max((long)0, experience + modifier);
	long new_value = get_level();
	if (new_value > old_value && new_value > 1) {
		add_toast("# Level Up!\n\n## You are now level " + std::to_string(new_value) + "\nYou gain +1 to maximum stamina, skill, speed, sneak, and luck.");
	}
}

bool player::is_dead() {
	return (stamina <= 0);
}

bool player::time_up() {
	return (days <= 0);
}

void player::add_skill(long modifier) {
	skill = std::max((long)0, skill + modifier);
	skill = std::min(skill, max_skill());
}

bool player::test_luck()
{
	bool result = (dice() + dice() <= luck);
	luck = std::max((long)0, luck - 1);
	return result;
}

bool player::test_stamina() {
	return(dice() + dice() <= stamina);
}

bool player::test_skill() {
	return(dice() + dice() <= skill);
}

bool player::test_speed() {
	return(dice() + dice() <= speed);
}

bool player::test_experience() {
	return(dice() + dice() * 4 <= experience);
}

void player::add_luck(long modifier) {
	luck = std::max((long)0, luck + modifier);
	luck = std::min(luck, max_luck());
}

void player::add_notoriety(long modifier) {
	notoriety = std::max((long)0, notoriety + modifier);
}

void player::add_sneak(long modifier) {
	sneak = std::max((long)0, sneak + modifier);
	sneak = std::min(sneak, max_sneak());
}

void player::add_speed(long modifier) {
	speed = std::max((long)0, speed + modifier);
	speed = std::min(speed, max_speed());
}

void player::add_gold(long modifier) {
	gold = std::max((long)0, gold + modifier);
	gold = std::min(gold, max_gold());
}

void player::add_silver(long modifier) {
	silver = std::max((long)0, silver + modifier);
	silver = std::min(silver, max_silver());
}

// Remove a ration point, or subtract 2 stamina if none left -
// returns false if out of rations (so warnings can be displayed)
bool player::eat_ration() {
	if (rations-- < 1) {
		if (stamina > 3) {
			add_stamina(-2);
			add_toast("You are hungry. You have lost stamina as you do not have enough rations to survive. Not eating will eventually lower your stamina to 1 point.");
		}
		rations = 0;
		return false;
	}
	return true;
}

void player::add_rations(long modifier) {
	rations = std::max((long)0, rations + modifier);
}

void player::add_mana(long modifier) {
	mana = std::max((long)0, mana + modifier);
	mana = std::min(mana, max_mana());
}

// Remove a day and return false if the time is now up

bool player::remove_day() {
	if (days-- < 1) {
		days = 0;
		return false;
	}
	return true;
}

long bonuses_numeric(int type, player_race R, player_profession P) {
	long mod_stm = 0, mod_skl = 0, mod_luk = 0, mod_snk = 0, mod_spd = 0, bonus = 0;
	
	switch (type)
	{
		case 1:
			if (R==race_human)		mod_stm+=1;
			if (R==race_elf)		mod_stm+=-1;
			if (R==race_orc)   		mod_stm+=4;
			if (R==race_lesser_orc)		mod_stm+=-1;
			if (R==race_goblin)		mod_stm+=-1;
			if (R==race_dwarf)		mod_stm+=1;
			if (R==race_barbarian)		mod_stm+=3;
			if (R==race_dark_elf)		mod_stm+=-1;

			if (P==prof_warrior)		mod_stm+=3;
			if (P==prof_mercenary)		mod_stm+=2;
			bonus = mod_stm;
		break;
		case 2:
			if (R==race_human)		mod_skl+=1;
			if (R==race_elf)		mod_skl+=2;
			if (R==race_orc)   		mod_skl+=-1;
			if (R==race_lesser_orc)		mod_skl+=-2;
			if (R==race_goblin)		mod_skl+=-1;
			if (R==race_dwarf)		mod_skl+=1;
			if (R==race_barbarian)		mod_skl+=-1;
			if (R==race_dark_elf)		mod_skl+=+2;

			if (P==prof_wizard)		mod_skl+=3;
			if (P==prof_assassin)		mod_skl+=2;
			if (P==prof_mercenary)		mod_skl+=1;
			bonus = mod_skl;
		break;
		case 3:
			if (R==race_human)		mod_luk+=0;
			if (R==race_elf)		mod_luk+=-2;
			if (R==race_orc)   		mod_luk+=-1;
			if (R==race_lesser_orc)		mod_luk+=2;
			if (R==race_goblin)		mod_luk+=1;
			if (R==race_dwarf)		mod_luk+=0;
			if (R==race_barbarian)		mod_luk+=0;
			if (R==race_dark_elf)		mod_luk+=-4;

			if (P==prof_woodsman)	mod_luk+=2;
			bonus = mod_luk;
		break;
		case 4:
			if (R==race_human)		mod_snk+=-1;
			if (R==race_elf)		mod_snk+=0;
			if (R==race_orc)   		mod_snk+=-1;
			if (R==race_lesser_orc)		mod_snk+=2;
			if (R==race_goblin)		mod_snk+=1;
			if (R==race_dwarf)		mod_snk+=0;
			if (R==race_barbarian)		mod_snk+=-1;
			if (R==race_dark_elf)		mod_snk+=2;

			if (P==prof_thief)		mod_snk+=3;
			if (P==prof_assassin)		mod_snk+=1;
			bonus = mod_snk;
		break;
		case 5:
			if (R==race_human)		mod_spd+=-1;
			if (R==race_elf)		mod_spd+=1;
			if (R==race_orc)   		mod_spd+=-1;
			if (R==race_lesser_orc)		mod_spd+=0;
			if (R==race_goblin)		mod_spd+=0;
			if (R==race_dwarf)		mod_spd+=-2;
			if (R==race_barbarian)		mod_spd+=-1;
			if (R==race_dark_elf)		mod_spd+=1;

			if (P==prof_woodsman)		mod_spd+=1;
			bonus = mod_spd;
		break;
	}

	return bonus;

}

std::string bonuses(int type, player_race R, player_profession P) {
	long bonus = bonuses_numeric(type, R, P);
	if (bonus > 0) {
		return fmt::format(" (+{})", bonus);
	} else if (bonus == 0) {
		return " ";
	} else {
		return fmt::format(" ({})", bonus);
	}
}

long player::max_stamina() {
	long level = get_level(), special = 0;
	if (profession == prof_warrior) {
		special += 5;
        } else if (race == race_barbarian || race == race_orc) {
		special += 4;
	}
	return 20 + level + special;
}

long player::max_skill() {
	long level = get_level();
	return 18 + level;
}

long player::max_luck() {
	long level = get_level();
	return 16 + level;
}

long player::max_sneak() {
	long level = get_level(), special = 0;
	if (profession == prof_thief) {
		special += 3;
	} else if (profession == prof_assassin) {
		special += 2;
	}
	if (race == race_lesser_orc) {
		special += 3;
	}
	return 15 + level + special;
}

long player::max_speed() {
	long level = get_level();
	return 20 + level;
}

long player::max_gold() {
	long level = get_level();
	return 50 + (level * 10);
}

long player::max_silver() {
	long level = get_level();
	return 100 + (level * 10);
}

void player::add_toast(const std::string& message) {
	if (toasts.size() < 8) {
		toasts.push_back(message);
	}
}

std::vector<std::string> player::get_toasts() {
	std::vector<std::string> return_toast = toasts;
	toasts.clear();
	return return_toast;
}
