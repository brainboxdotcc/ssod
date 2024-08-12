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
#include <string>
#include <vector>
#include <deque>
#include <dpp/dpp.h>
#include <ssod/game_enums.h>
#include <ssod/ssod.h>
#include <ssod/database.h>

struct rated_item {
	std::string name{};
	long rating{1};
};

enum game_state {
	state_roll_stats,
	state_pick_magic,
	state_name_player,
	state_play,
};

struct enemy {
	std::string name;
	long stamina{};
	long skill{};
	long armour{};
	long weapon{};
	long xp_value{};
};

enum combat_stance {
	OFFENSIVE = 1,
	DEFENSIVE = 2,
};

enum combat_strike {
	CUTTING = 1,
	PIERCING = 2,
};

struct toast {
	std::string message;
	std::string image;
};

/**
 * @brief Represents a player. These are loaded from the database, cached to RAM
 * and frequently saved back to the database.
 */
struct player {
	/**
	 * @brief The current player state in the game's state machine
	 */
	game_state state;
	/**
	 * @brief The last D++ app command event for the player
	 */
	dpp::interaction_create_t event;
	/**
	 * @brief Current combat stance
	 */
	combat_stance stance;
	/**
	 * @brief Current combat attack type
	 */
	combat_strike attack;
	/**
	 * @brief True if in combat
	 */
	bool in_combat{};
	/**
	 * @brief True if in inventory
	 */
	bool in_inventory{};
	/**
	 * @brief True if in bank
	 */
	bool in_bank{};
	/**
	 * @brief True if in the grimoire
	 */
	bool in_grimoire{};
	/**
	 * @brief True if in the campfire
	 */
	bool in_campfire{};
	/**
	 * @brief True if in PVP picker
	 */
	bool in_pvp_picker{};
	/**
	 * @brief Inventory, spells or herbs have changed
	 */
	bool inv_change{};
	/**
	 * @brief True if next hit in pve combat is to be a critical
	 */
	bool next_crit{};
	/**
	 * @brief Who has challenged this user to pvp
	 */
	dpp::snowflake challenged_by{};
	/**
	 * @brief Current fragment of a paragraph with combat
	 */
	int after_fragment;
	/**
	 * @brief Combatant for pve combat
	 */
	enemy combatant;
	/**
	 * @brief Player name
	 */
	std::string name;
	/**
	 * @brief Player race
	 */
	player_race race;
	/**
	 * @brief Player profession
	 */
	player_profession profession;
	/**
	 * @brief Player gender
	 */
	std::string gender;
	/**
	 * @brief Player stamina
	 * Zero stamina = dead
	 */
	long stamina;
	/**
	 * @brief Player skill
	 */
	long skill;
	/**
	 * @brief Player luck
	 */
	long luck;
	/**
	 * @brief Player sneak
	 */
	long sneak;
	/**
	 * @brief Player speed
	 */
	long speed;
	/**
	 * @brief Player silver
	 */
	long silver;
	/**
	 * @brief Player gold
	 */
	long gold;
	/**
	 * @brief Player rations
	 */
	long rations;
	/**
	 * @brief Player experience
	 */
	long experience;
	/**
	 * @brief Player notoriety
	 */
	long notoriety;
	/**
	 * @brief Player days
	 */
	long days;
	/**
	 * @brief Player scroll count
	 */
	long scrolls;
	/**
	 * @brief Player paragraph
	 */
	uint32_t paragraph;
	/**
	 * @brief Player armour
	 */
	rated_item armour;
	/**
	 * @brief Player weapon
	 */
	rated_item weapon;
	/**
	 * @brief Player inventory
	 */
	std::vector<stacked_item> possessions;
	/**
	 * @brief Player notes
	 */
	std::vector<std::string> notes;
	/**
	 * @brief Toast messages
	 */
	std::vector<toast> toasts;
	/**
	 * @brief Player spells
	 */
	std::vector<item> spells;
	/**
	 * @brief Player herbs
	 * (used as components for spell casting)
	 */
	std::vector<item> herbs;
	/**
	 * @brief Time player last accessed
	 */
	time_t last_use;
	/**
	 * @brief Time player last landed a blow in combat
	 */
	time_t last_strike;
	/**
	 * @brief If non-zero, player can't move until this time
	 */
	time_t pinned;
	/**
	 * @brief If non-zero, player muted until this time
	 */
	time_t muted;
	/**
	 * @brief Spellcasting mana
	 * (stops spell spamming in pvp)
	 */
	long mana;
	/**
	 * @brief Mana last time_t value when mana was ticked up
	 */
	time_t mana_tick;
	/**
	 * @brief Current inventory page number
	 */
	size_t inventory_page;
	/**
	 * @brief Previous paragraphs travelled through that:
	 * 1) did not contain combat
	 * 2) contained a choice and displayed text
	 * If the player is offered a do-over (normal users get two per day,
	 * premium users get six) then they can restart from the start of their
	 * breadcrumb trail. This puts them back somewhere safe where they can
	 * potentially explore a different path. If they have no do-overs,
	 * they must restart from the start. Do overs are called resurrections.
	 */
	std::deque<long> breadcrumb_trail;

	time_t last_resurrect;

	int g_dice{0};

	/**
	 * @brief Destroy the player object
	 * 
	 */
	~player();
	player(bool reroll = false);
	player(dpp::snowflake user_id, bool get_backup = false);
	bool save(dpp::snowflake user_id, bool put_backup = false);
	dpp::snowflake get_user();
	void strike();
	void reset_to_spawn_point();
	bool sneak_test(long monster_sneak);
	void add_stamina(long modifier);
	dpp::task<void> add_experience(long modifier);
	void blocking_add_experience(long modifier);
	bool is_dead();
	bool time_up();
	void add_skill(long modifier);

	bool test_luck();
	bool test_stamina();
	bool test_skill();
	bool test_speed();
	bool test_experience();

	void add_luck(long modifier);
	void add_sneak(long modifier);
	void add_speed(long modifier);
	void add_gold(long modifier);
	void add_silver(long modifier);
	void add_notoriety(long modifier);
	void add_mana(long modifier);
	bool eat_ration();
	void add_rations(long modifier);
	bool remove_day();

	long max_stamina();
	long max_skill();
	long max_luck();
	long max_sneak();
	long max_speed();
	long max_gold();
	long max_silver();
	long max_mana();
	long max_rations();
	long max_crits();

	void tick_mana();

	void add_toast(const toast& message);
	std::vector<toast> get_toasts();

	void add_flag(const std::string flag, long paragraph = -1);
	bool has_flag(const std::string flag, long paragraph = -1);

	bool has_herb(std::string herb_name);
	bool has_component_herb(const std::string& spell);
	bool has_spell(std::string spell_name);
	bool has_possession(std::string name);
	std::vector<stacked_item> possessions_page(size_t page_number);
	size_t max_inventory_slots();

	bool drop_possession(const item& i);
	void pickup_possession(item i);
	void pickup_possession(stacked_item i);
	dpp::task<void> drop_everything();
	bool drop_spell(const item& i);
	bool drop_herb(const item& i);

	long get_level() const;
	void death_xp_loss();
	double get_percent_of_current_level();
	long xp_worth();

	dpp::task<dpp::message> get_registration_message(class dpp::cluster& cluster, const dpp::interaction_create_t &event);
	dpp::task<dpp::message> get_magic_selection_message(dpp::cluster& cluster, const dpp::interaction_create_t &event);

	bool convert_rations(const item& i);

	void run_inventory_insert_query(std::string &query, const std::vector<db::parameter_type> &p);

	void insert_owned_list(dpp::snowflake user_id, const std::vector<item>& items);
	void insert_owned_list(dpp::snowflake user_id, const std::vector<stacked_item>& items);
};

using player_list = std::unordered_map<dpp::snowflake, player>;

bool player_is_registering(dpp::snowflake user_id);
uint64_t get_active_player_count();
player get_registering_player(const dpp::interaction_create_t& event);
void update_registering_player(const dpp::interaction_create_t& event, player p);
void move_from_registering_to_live(const dpp::interaction_create_t& event, player p);
dpp::task<bool> player_is_live(const dpp::interaction_create_t& event);
player get_live_player(const dpp::interaction_create_t& event, bool update_event = true);
void update_live_player(const dpp::interaction_create_t& event, player p);
dpp::task<void> delete_live_player(const dpp::interaction_create_t& event);
dpp::task<void> cleanup_idle_live_players();
void cleanup_idle_reg_players();
void unload_live_player(uint64_t id);

long bonuses_numeric(int type, player_race R, player_profession P);
std::string bonuses(int type, player_race R, player_profession P);
