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
#include <span>

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
	-100000000,
	0,     // 1
	10,    // 2
	20,    // 3
	40,    // 4
	80,    // 5
	160,   // 6
	250,   // 7
	500,   // 8
	550,   // 9
	1000,  // 10
	1500,  // 11
	2000,  // 12
	2500,  // 13
	3000,  // 14
	3500,  // 15
	4000,  // 16
	4500,  // 17
	5000,  // 18
	5500,  // 19
	6000,  // 20
	6500,  // 21
	7500,  // 21
	8500,  // 22
	9500,  // 23
	10000, // 24
	12000, // 25
	14000, // 26
	16000, // 27
	18000, // 28
	20000, // 29
	25000, // 30
	35000, // 31
	50000, // 32
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
	auto rs = db::query("SELECT * FROM cache_purge_queue ORDER BY id");
	std::lock_guard<std::mutex> l(live_list_lock);
	if (!rs.empty()) {
		for (const auto &row : rs) {
			auto p = live_players.find(atoll(row.at("user_id")));
			if (p != live_players.end()) {
				p->second.last_use = 0;
			}
		}
		db::query("DELETE FROM cache_purge_queue");
	}
	player_list copy;
	time_t ten_mins_ago = time(nullptr) - 3600;
	for (const std::pair<const dpp::snowflake, player> &pair: live_players) {
		if (pair.second.last_use > ten_mins_ago) {
			copy[pair.first] = pair.second;
		}
	}
	live_players = copy;
}

void unload_live_player(uint64_t id) {
	std::lock_guard<std::mutex> l(live_list_lock);
	player_list copy;
	for (const std::pair<const dpp::snowflake, player>& pair : live_players) {
		if (pair.first != id) {
			copy[pair.first] = pair.second;
		}
	}
	live_players = copy;
}

void cleanup_idle_reg_players() {
	std::lock_guard<std::mutex> l(reg_list_lock);
	player_list copy;
	time_t ten_mins_ago = time(nullptr) - 3600;
	for (const std::pair<const dpp::snowflake, player> &pair: registering_players) {
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
		.set_title(_("NEWCHAR", event))
		.set_footer(dpp::embed_footer{ 
			.text = _("NEWPLAY", event, event.command.usr.format_username()), 
			.icon_url = cluster.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(_("WELCOME_MSG", event, event.command.usr.get_mention()))
		.add_field(_("STAMINA", event), std::to_string(stamina) + bonuses(1, race, profession), true)
		.add_field(_("SKILL", event), std::to_string(skill) + bonuses(2, race, profession), true)
		.add_field(_("LUCK", event), std::to_string(luck) + bonuses(3, race, profession), true)
		.add_field(_("SNEAK", event), std::to_string(sneak) + bonuses(4, race, profession), true)
		.add_field(_("SPEED", event), std::to_string(speed) + bonuses(5, race, profession), true)
		.add_field(_("GOLD", event), std::to_string(gold), true)
		.add_field(_("SILVER", event), std::to_string(silver), true)
		.add_field(_("RATIONS", event), std::to_string(rations), true)
		.add_field(_("NOTORIETY", event), std::to_string(notoriety), true)
		.add_field(_("ARMOUR", event), fmt::format("{} ({} {})", armour.name, _("RATING", event), armour.rating), true)
		.add_field(_("WEAPON", event), fmt::format("{} ({} {})", weapon.name, _("RATING", event), weapon.rating), true)
		.set_image(file);

		dpp::component race_select_menu, profession_select_menu, gender_select_menu;
		race_select_menu.set_type(dpp::cot_selectmenu)
			.set_min_values(1)
			.set_max_values(1)
			.set_required(true)
			.set_placeholder(_("SELECTRACE", event))
			.set_id(security::encrypt("select_player_race"))
			.add_select_option(dpp::select_option(_("HUMAN", event), "1", _("HUMAN_DESC", event)).set_default(race == race_human))
			.add_select_option(dpp::select_option(_("ELF", event), "2", _("ELF_DESC", event)).set_default(race == race_elf))
			.add_select_option(dpp::select_option(_("ORC", event), "3", _("ORC_DESC", event)).set_default(race == race_orc))
			.add_select_option(dpp::select_option(_("DWARF", event), "4", _("DWARF_DESC", event)).set_default(race == race_dwarf))
			.add_select_option(dpp::select_option(_("LORC", event), "5", _("LORC_DESC", event)).set_default(race == race_lesser_orc))
			.add_select_option(dpp::select_option(_("BARBARIAN", event), "6", _("BARBARIAN_DESC", event)).set_default(race == race_barbarian))
			.add_select_option(dpp::select_option(_("GOBLIN", event), "7", _("GOBLIN_DESC", event)).set_default(race == race_goblin))
			.add_select_option(dpp::select_option(_("DARKELF", event), "8", _("DARKELF_DESC", event)).set_default(race == race_dark_elf));
		profession_select_menu.set_type(dpp::cot_selectmenu)
			.set_min_values(1)
			.set_max_values(1)
			.set_placeholder(_("SELECTPROFESSION", event))
			.set_required(true)
			.set_id(security::encrypt("select_player_profession"))
			.add_select_option(dpp::select_option(_("WARRIOR", event), "1", _("WARRIOR_DESC", event)).set_default(profession == prof_warrior))
			.add_select_option(dpp::select_option(_("WIZARD", event), "2", _("WIZARD_DESC", event)).set_default(profession == prof_wizard))
			.add_select_option(dpp::select_option(_("THIEF", event), "3", _("THIEF_DESC", event)).set_default(profession == prof_thief))
			.add_select_option(dpp::select_option(_("WOODSMAN", event), "4", _("WOODSMAN_DESC", event)).set_default(profession == prof_woodsman))
			.add_select_option(dpp::select_option(_("ASSASSIN", event), "5", _("ASSASSIN_DESC", event)).set_default(profession == prof_assassin))
			.add_select_option(dpp::select_option(_("MERCENARY", event), "6", _("MERCENARY_DESC", event)).set_default(profession == prof_mercenary));
		gender_select_menu.set_type(dpp::cot_selectmenu)
			.set_min_values(1)
			.set_max_values(1)
			.set_placeholder(_("SELGENDER", event))
			.set_required(true)
			.set_default_value("male")
			.set_id(security::encrypt("select_player_gender"))
			.add_select_option(dpp::select_option(_("male", event), "male").set_default(gender == "male"))
			.add_select_option(dpp::select_option(_("female", event), "female").set_default(gender == "female"));

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
				.set_label(_("REROLL", event))
				.set_style(dpp::cos_danger)
				.set_emoji("🎲")
			)
			.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id(security::encrypt("player_herb_spell_selection"))
				.set_label(_("CONTINUE", event))
				.set_style(dpp::cos_success)
				.set_emoji("➡️")
			)
			.add_component(help_button(event))
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
		.set_title(_("SELMAGIC", event))
		.set_footer(dpp::embed_footer{ 
			.text = _("NEWPLAY", event, event.command.usr.format_username()), 
			.icon_url = cluster.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(_("MAGIC_MSG", event, herbs.size(), 3, spells.size(), max_spells));

	dpp::component herb_select_menu, spell_select_menu;
	herb_select_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_max_values(3)
		.set_placeholder(_("SELHERB", event))
		.set_id(security::encrypt("select_player_herbs"))
		.add_select_option(dpp::select_option(_("HARTLEAF", event), "hartleaf", _("HARTLEAFD", event)).set_default(has_herb("hartleaf")))
		.add_select_option(dpp::select_option(_("ELFBANE", event), "elfbane", _("ELFBANED", event)).set_default(has_herb("elfbane")))
		.add_select_option(dpp::select_option(_("MONKGRASS", event), "monkgrass", _("MONKGRASSD", event)).set_default(has_herb("monkgrass")))
		.add_select_option(dpp::select_option(_("FIRESEEDS", event), "fireseeds", _("FIRESEEDSD", event)).set_default(has_herb("fireseeds")))
		.add_select_option(dpp::select_option(_("WOODWEED", event), "woodweed", _("WOODWEEDD", event)).set_default(has_herb("woodweed")))
		.add_select_option(dpp::select_option(_("BLIDVINES", event), "blidvines", _("BLIDVINESD", event)).set_default(has_herb("blidvines")))
		.add_select_option(dpp::select_option(_("STICKWART", event), "stickwart", _("STICKWARTD", event)).set_default(has_herb("stickwart")))
		.add_select_option(dpp::select_option(_("SPIKEGRASS", event), "spikegrass", _("SPIKEGRASSD", event)).set_default(has_herb("spikegrass")))
		.add_select_option(dpp::select_option(_("HALLUCINOGEN", event), "hallucinogen", _("HALLUCINOGEND", event)).set_default(has_herb("hallucinogen")))
		.add_select_option(dpp::select_option(_("WIZARDSIVY", event), "wizardsivy", _("WIZARDSIVYD", event)).set_default(has_herb("wizardsivy")))
		.add_select_option(dpp::select_option(_("ORCWEED", event), "orcweed", _("ORCWEEDD", event)).set_default(has_herb("orcweed")));
	spell_select_menu.set_type(dpp::cot_selectmenu)
		.set_min_values(0)
		.set_placeholder(_("SELSPELL", event))
		.set_id(security::encrypt("select_player_spells"));
	/* Fill spell select menu only with spells applicable to the chosen herbs up to a max of 25 choices */
	const std::vector<dpp::select_option> all_spells{
		dpp::select_option(_("FIRE", event), "fire", _("FIRED", event)),
		dpp::select_option(_("WATER", event), "water", _("WATERD", event)),
		dpp::select_option(_("LIGHT", event), "light", _("LIGHTD", event)),
		dpp::select_option(_("FLY", event), "fly", _("FLYD", event)),
		dpp::select_option(_("STRENGTH", event), "strength", _("STRENGTHD", event)),
		dpp::select_option(_("XRAY", event), "xray", _("XRAYD", event)),
		dpp::select_option(_("BOLT", event), "bolt", _("BOLTD", event)),
		dpp::select_option(_("FASTHANDS", event), "fasthands", _("FASTHANDSD", event)),
		dpp::select_option(_("THUNDERBOLT", event), "thunderbolt", _("THUNDERBOLTD", event)),
		dpp::select_option(_("STEAL", event), "steal", _("STEALD", event)),
		dpp::select_option(_("SHIELD", event), "shield", _("SHIELDD", event)),
		dpp::select_option(_("JUMP", event), "jump", _("JUMPD", event)),
		dpp::select_option(_("OPEN", event), "open", _("OPEND", event)),
		dpp::select_option(_("SPOT", event), "spot", _("SPOTD", event)),
		dpp::select_option(_("SNEAK", event), "sneak", _("SNEAKD", event)),
		dpp::select_option(_("ESP", event), "esp", _("ESPD", event)),
		dpp::select_option(_("RUN", event), "run", _("RUND", event)),
		dpp::select_option(_("INVISIBLE", event), "invisible", _("INVISIBLED", event)),
		dpp::select_option(_("SHRINK", event), "shrink", _("SHRINKD", event)),
		dpp::select_option(_("GROW", event), "grow", _("GROWD", event)),
		dpp::select_option(_("AIR", event), "air", _("AIRD", event)),
		dpp::select_option(_("ANIMAL", event), "animalcommunication", _("ANIMALD", event)),
		dpp::select_option(_("WEAPONSKILL", event), "weaponskill", _("WEAPONSKILLD", event)),
		dpp::select_option(_("HEALING", event), "healing", _("HEALINGD", event)),
		dpp::select_option(_("WOODSMANSHIP", event), "woodsmanship", _("WOODSMANSHIPD", event)),
		dpp::select_option(_("NIGHTVISION", event), "nightvision", _("NIGHTVISIOND", event)),
		dpp::select_option(_("HEATEYES", event), "heateyes", _("HEATEYESD", event)),
		dpp::select_option(_("DECIPHER", event), "decipher", _("DECIPHERD", event)),
		dpp::select_option(_("DETECT", event), "detect", _("DETECTD", event)),
		dpp::select_option(_("TRACKING", event), "tracking", _("TRACKINGD", event)),
		dpp::select_option(_("ESPSURGE", event), "espsurge", _("ESPSURGED", event)),
		dpp::select_option(_("AFTERIMAGE", event), "afterimage", _("AFTERIMAGED", event)),
		dpp::select_option(_("PSYCHISM", event), "psychism", _("PSYCHISMD", event)),
		dpp::select_option(_("SPIRITWALK", event), "spiritwalk", _("SPIRITWALKD", event)),
		dpp::select_option(_("GROWWEAPON", event), "growweapon", _("GROWWEAPOND", event)),
	};

	size_t spell_count = 0;
	for (auto spell : all_spells) {
		if (has_component_herb(spell.value)) {
			spell_select_menu.add_select_option(spell.set_default(has_spell(spell.value)));
			spell_count++;
		}
	}
	if (!spell_count) {
		spell_select_menu.add_select_option(dpp::select_option(_("SELECTONEHERB", event), "0", _("MUSTCHOOSE", event)));
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
				.set_label(_("CONTINUE", event))
				.set_style(dpp::cos_success)
				.set_emoji("➡️")
			)
			.add_component(help_button(event))
		).set_flags(dpp::m_ephemeral);
}

std::vector<item> player::possessions_page(size_t page_number) {
	if (possessions.size() < page_number * 25) {
		return {};
	}
	std::vector<item> nv;
	for (size_t i = page_number * 25; i < (page_number + 1) * 25; ++i) {
		if (i < possessions.size()) {
			nv.emplace_back(possessions[i]);
		}
	}
	return nv;
}

size_t player::max_inventory_slots() {
	int pages_max = 1;
	if (has_flag("horse")) {
		pages_max++;
	}
	if (has_flag("pack")) {
		pages_max++;
	}
	if (has_flag("saddlebags")) {
		pages_max++;
	}
	if (has_flag("steamcopter")) {
		pages_max += 3;
	}
	return pages_max * 25;
}

player::player(bool reroll) :
	state(state_roll_stats), in_combat(false), in_inventory(false), in_bank(false), in_pvp_picker(false), inv_change(false),
	challenged_by(0), after_fragment(0), race(player_race::race_error), profession(player_profession::prof_error), stamina(0),
	skill(0), luck(0), sneak(0), speed(0), silver(0), gold(0), rations(0), experience(0), notoriety(0), days(0), scrolls(0),
	last_use(0), last_strike(0), pinned(0), muted(0), mana(0), mana_tick(0), inventory_page(0) {
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
		/* We don't drop quest items */
		sale_info value = get_sale_info(i.name);
		if (!value.quest_item && dpp::lowercase(i.name) != "scroll") {
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
	std::string f{" [" + flag + std::to_string(paragraph) + "]"};
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
		add_toast(_("LEVELUP", event));
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
			add_toast(_("HUNGER", event));
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
