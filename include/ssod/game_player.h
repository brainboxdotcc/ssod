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

struct player {
	game_state state;
	dpp::interaction_create_t event;
	player_race race;
	player_profession profession;
	player_profession X;
	long stamina;
	long skill;
	long luck;
	long sneak;
	long speed;
	long silver;
	long gold;
	long rations;
	long experience;
	long notoriety;
	long days;
	long scrolls;
	std::string paragraph;
	rated_item armour;
	rated_item weapon;
	std::vector<item> possessions;
	std::vector<std::string> notes;
	std::vector<item> spells;
	std::vector<item> herbs;
	std::string gotfrom;
	time_t last_use;
	time_t last_strike;
	time_t pinned;
	time_t muted;
	long mana;
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

	bool has_herb(const std::string herb_name);
	bool has_component_herb(const std::string& spell);

	dpp::message get_registration_message(class dpp::cluster& cluster, const dpp::interaction_create_t &event);
	dpp::message get_magic_selection_message(dpp::cluster& cluster, const dpp::interaction_create_t &event);
};

using player_list = std::unordered_map<dpp::snowflake, player>;

bool player_is_registering(dpp::snowflake user_id);
player get_registering_player(const dpp::interaction_create_t& event);
void update_registering_player(const dpp::interaction_create_t& event, player p);
bool player_is_live(dpp::snowflake user_id);
player get_live_player(dpp::snowflake user_id);

long bonuses_numeric(int type, player_race R, player_profession P);
std::string bonuses(int type, player_race R, player_profession P);
