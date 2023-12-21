#pragma once
#include <string>
#include <vector>
#include <dpp/dpp.h>
#include <ssod/game_enums.h>

struct item {
	std::string name{};
	std::string flags{};
};

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
};

enum combat_stance {
	OFFENSIVE = 1,
	DEFENSIVE = 2,
};

enum combat_strike {
	CUTTING = 1,
	PIERCING = 2,
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
	std::vector<item> possessions;
	/**
	 * @brief Player notes
	 */
	std::vector<std::string> notes;
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
	 * @brief Player flag list
	 * State set by paragraph
	 */
	std::string gotfrom;
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
	 * @brief Mana recharge rate
	 */
	time_t mana_tick;

	~player();
	player(bool reroll = false);
	player(dpp::snowflake user_id, bool get_backup = false);
	bool save(dpp::snowflake user_id, bool put_backup = false);
	void strike();
	void reset_to_spawn_point();
	std::string get_flags();
	bool sneak_test(long monster_sneak);
	void add_stamina(long modifier);
	void add_experience(long modifier);
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
	bool eat_ration();
	void add_rations(long modifier);
	bool remove_day();

	void add_flag(const std::string flag, long paragraph);
	bool has_flag(const std::string flag, long paragraph);

	bool has_herb(const std::string herb_name);
	bool has_component_herb(const std::string& spell);
	bool has_spell(const std::string spell_name);
	bool has_possession(const std::string name);

	bool drop_possession(const item& i);
	bool drop_spell(const item& i);
	bool drop_herb(const item& i);

	dpp::message get_registration_message(class dpp::cluster& cluster, const dpp::interaction_create_t &event);
	dpp::message get_magic_selection_message(dpp::cluster& cluster, const dpp::interaction_create_t &event);
};

using player_list = std::unordered_map<dpp::snowflake, player>;

bool player_is_registering(dpp::snowflake user_id);
player get_registering_player(const dpp::interaction_create_t& event);
void update_registering_player(const dpp::interaction_create_t& event, player p);
void move_from_registering_to_live(const dpp::interaction_create_t& event, player p);
bool player_is_live(const dpp::interaction_create_t& event);
player get_live_player(const dpp::interaction_create_t& event);
void update_live_player(const dpp::interaction_create_t& event, player p);

long bonuses_numeric(int type, player_race R, player_profession P);
std::string bonuses(int type, player_race R, player_profession P);
