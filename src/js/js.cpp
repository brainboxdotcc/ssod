/************************************************************************************
 * 
 * Sporks, the learning, scriptable Discord bot!
 *
 * Copyright 2019 Craig Edwards <support@sporks.gg>
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

#include <dpp/dpp.h>
#include <dpp/json.h>
#include <ssod/js.h>
#include <streambuf>
#include <cstdio>
#include "duktape.h"
#include <ssod/paragraph.h>
#include <ssod/database.h>
#include <ssod/achievement.h>

namespace js {

dpp::cluster* bot = nullptr;

void sandbox_fatal(void *udata, const char *msg);

class exit_exception : public std::exception {
};

static paragraph& duk_get_udata(duk_context* ctx) {
	duk_memory_functions funcs;
	duk_get_memory_functions(ctx, &funcs);
	return *((paragraph*)funcs.udata);
}

static void define_func(duk_context* ctx, const std::string &name, duk_c_function func, int nargs) {
	duk_push_string(ctx, name.c_str());
	duk_push_c_function(ctx, func, nargs);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE);
}

static void define_number(duk_context* ctx, const std::string &name, duk_double_t num) {
	duk_push_string(ctx, name.c_str());
	duk_push_number(ctx, num);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE);
}

static void define_string(duk_context* ctx, const std::string &name, const std::string &value) {
	duk_push_string(ctx, name.c_str());
	duk_push_string(ctx, value.c_str());
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE);
}

static duk_ret_t js_print(duk_context *cx) {
	int argc = duk_get_top(cx);
	std::string output;
	if (argc < 1) {
		return 0;
	}
	for (int i = 0; i < argc; i++) {
		output.append(duk_to_string(cx, i - argc)).append(" ");
	}
	paragraph& p = duk_get_udata(cx);
	*(p.output) << output;
	return 0;
}

static duk_ret_t js_tag(duk_context *cx) {
	int argc = duk_get_top(cx);
	std::string output;
	if (argc < 1) {
		return 0;
	}
	for (int i = 0; i < argc; i++) {
		output.append(duk_to_string(cx, i - argc)).append(" ");
	}
	paragraph& p = duk_get_udata(cx);
	if (p.id == 0) {
		bot->log(dpp::ll_warning, "JS tag(): Cannot recursively execute a script tag inside tag()!");
		return 0;
	}
	paragraph inner(output, *p.cur_player);
	*p.output << inner.text;
	p.links += inner.links;
	for (const auto& nav : inner.navigation_links) {
		p.navigation_links.push_back(nav);
	}
	if (inner.g_dice) {
		p.g_dice = inner.g_dice;
	}
	return 0;
}

[[maybe_unused]]
static void duk_build_object(duk_context* cx, const std::map<std::string, std::string> &strings, const std::map<std::string, bool> &bools) {
	duk_idx_t obj_idx = duk_push_bare_object(cx);
	for (const auto & string : strings) {
		duk_push_string(cx, string.first.c_str());
		duk_push_string(cx, string.second.c_str());
		duk_put_prop(cx, obj_idx);
	}
	for (const auto & i : bools) {
		duk_push_string(cx, i.first.c_str());
		duk_push_boolean(cx, i.second);
		duk_put_prop(cx, obj_idx);
	}
}

static duk_ret_t js_exit(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS exit(): incorrect number of parameters: " + std::to_string(argc));
	}
	throw exit_exception();
}

void init(dpp::cluster& _bot) {
	bot = &_bot;
}

static duk_ret_t js_get_name(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_name: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_string(cx, p.cur_player->name.c_str());
	return 1;
}

static duk_ret_t js_get_race(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_race: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx, (duk_double_t)p.cur_player->race);
	return 1;
}

static duk_ret_t js_get_profession(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_profession: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->profession);
	return 1;
}

static duk_ret_t js_get_gender(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_gender: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_string(cx, p.cur_player->gender.c_str());
	return 1;
}

static duk_ret_t js_get_stamina(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_stamina: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx, (duk_double_t)p.cur_player->stamina);
	return 1;
}

static duk_ret_t js_get_skill(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_skill: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx, (duk_double_t)p.cur_player->skill);
	return 1;
}

static duk_ret_t js_get_luck(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_luck: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx, (duk_double_t)p.cur_player->luck);
	return 1;
}

static duk_ret_t js_get_sneak(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_sneak: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->sneak);
	return 1;
}

static duk_ret_t js_get_speed(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_speed: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->speed);
	return 1;
}

static duk_ret_t js_get_silver(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_silver: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->silver);
	return 1;
}

static duk_ret_t js_get_gold(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_gold: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->gold);
	return 1;
}

static duk_ret_t js_get_rations(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_rations: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->rations);
	return 1;
}

static duk_ret_t js_get_experience(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_experience: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->experience);
	return 1;
}

static duk_ret_t js_get_level(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_experience: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->get_level());
	return 1;
}

static duk_ret_t js_get_notoriety(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_notoriety: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->notoriety);
	return 1;
}

static duk_ret_t js_get_days(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_days: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->days);
	return 1;
}

static duk_ret_t js_get_scrolls(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_scrolls: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->scrolls);
	return 1;
}

static duk_ret_t js_get_paragraph(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_paragraph: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->paragraph);
	return 1;
}

static duk_ret_t js_get_armour(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_armour: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->armour.rating);
	return 1;
}

static duk_ret_t js_get_weapon(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_weapon: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->weapon.rating);
	return 1;
}

static duk_ret_t js_get_last_use(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_last_use: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->last_use);
	return 1;
}

static duk_ret_t js_get_last_strike(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_last_strike: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->last_strike);
	return 1;
}

static duk_ret_t js_get_pinned(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_pinned: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->pinned);
	return 1;
}

static duk_ret_t js_get_muted(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_muted: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->muted);
	return 1;
}

static duk_ret_t js_get_mana(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_mana: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->mana);
	return 1;
}

static duk_ret_t js_get_mana_tick(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 0) {
		bot->log(dpp::ll_warning, "JS get_mana_tick: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	paragraph& p = duk_get_udata(cx);
	duk_push_number(cx,(duk_double_t)p.cur_player->mana_tick);
	return 1;
}

static duk_ret_t js_get_key(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS get_key: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	const char* key = duk_get_string(cx, 0);
	paragraph& p = duk_get_udata(cx);
	auto rs = db::query("SELECT kv_value FROM kv_store WHERE user_id = ? AND kv_key = ?", { p.cur_player->event.command.usr.id, key });
	if (rs.empty()) {
		duk_push_string(cx, "");
	} else {
		duk_push_string(cx, rs[0].at("kv_value").c_str());
	}
	return 1;
}

static duk_ret_t js_delete_key(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS delete_key: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	const char* key = duk_get_string(cx, 0);
	paragraph& p = duk_get_udata(cx);
	db::query("DELETE FROM kv_store WHERE user_id = ? AND kv_key = ?", { p.cur_player->event.command.usr.id, key });
	return 0;
}


static duk_ret_t js_set_key(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 2) {
		bot->log(dpp::ll_warning, "JS set_key: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_string(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_key: parameter 1 is not a string");
		return 0;
	}
	std::string v;
	if (duk_is_string(cx, -1)) {
		const char* value = duk_get_string(cx, -1);
		v = value ? value : "";
	} else {
		auto value = (double)duk_get_number(cx, -1);
		v = fmt::format("{0:f}", value);
	}
	const char* key = duk_get_string(cx, 0);
	paragraph& p = duk_get_udata(cx);
	db::query("INSERT INTO kv_store (user_id, kv_key, kv_value) VALUES(?, ?, ?) ON DUPLICATE KEY UPDATE kv_value = ?", { p.cur_player->event.command.usr.id, key, v, v });
	return 0;
}

static duk_ret_t js_toast(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 2) {
		bot->log(dpp::ll_warning, "JS toast: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_string(cx, 0)) {
		bot->log(dpp::ll_warning, "JS toast: parameter 1 is not a string");
		return 0;
	}
	if (!duk_is_string(cx, -1)) {
		bot->log(dpp::ll_warning, "JS toast: parameter 2 is not a string");
		return 0;
	}
	const char* text = duk_get_string(cx, 0);
	const char* image = duk_get_string(cx, -1);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_toast(toast{ .message = text, .image = image });
	return 0;
}


static duk_ret_t js_set_name(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_name: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_string(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_name: parameter 1 is not a number");
		return 0;
	}
	const char* new_value = duk_get_string(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->name = new_value;
	return 0;
}

static duk_ret_t js_set_race(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_race: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_race: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->race = (player_race)new_value;
	return 0;
}

static duk_ret_t js_set_profession(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_profession: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_profession: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->profession = (player_profession)new_value;
	return 0;
}

static duk_ret_t js_set_gender(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_gender: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_string(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_gender: parameter 1 is not a number");
		return 0;
	}
	const char* new_value = duk_get_string(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->gender = new_value;
	return 0;
}

static duk_ret_t js_set_stamina(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_stamina: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_stamina: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->stamina = new_value;
	return 0;
}

static duk_ret_t js_set_skill(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_skill: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_skill: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->skill = new_value;
	return 0;
}

static duk_ret_t js_set_luck(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_luck: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_luck: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->luck = new_value;
	return 0;
}

static duk_ret_t js_set_sneak(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_sneak: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_sneak: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->sneak = new_value;
	return 0;
}

static duk_ret_t js_set_speed(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_speed: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_speed: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->speed = new_value;
	return 0;
}

static duk_ret_t js_set_silver(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_silver: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_silver: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->silver = new_value;
	return 0;
}

static duk_ret_t js_set_gold(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_gold: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_gold: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->gold = new_value;
	return 0;
}

static duk_ret_t js_set_rations(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_rations: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_rations: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->rations = new_value;
	return 0;
}

static duk_ret_t js_set_experience(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_experience: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_experience: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->experience = new_value;
	return 0;
}

static duk_ret_t js_set_notoriety(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_notoriety: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_notoriety: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->notoriety = new_value;
	return 0;
}

static duk_ret_t js_set_days(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_days: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_days: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->days = new_value;
	return 0;
}

static duk_ret_t js_set_scrolls(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_scrolls: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_scrolls: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->scrolls = std::min(7L, std::max(0L, new_value));
	return 0;
}

static duk_ret_t js_set_paragraph(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_paragraph: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_paragraph: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->paragraph = new_value;
	return 0;
}

static duk_ret_t js_set_armour(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_armour: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_armour: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->armour.rating = new_value;
	return 0;
}

static duk_ret_t js_set_weapon(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_weapon: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_weapon: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->weapon.rating = new_value;
	return 0;
}

static duk_ret_t js_set_last_use(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_last_use: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_last_use: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->last_use = new_value;
	return 0;
}

static duk_ret_t js_set_last_strike(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_last_strike: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_last_strike: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->last_strike = new_value;
	return 0;
}

static duk_ret_t js_set_pinned(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_pinned: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_pinned: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->pinned = new_value;
	return 0;
}

static duk_ret_t js_set_muted(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_muted: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_muted: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->muted = new_value;
	return 0;
}

static duk_ret_t js_set_mana(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_mana: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_mana: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->mana = new_value;
	return 0;
}

static duk_ret_t js_set_mana_tick(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS set_mana_tick: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_mana_tick: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->mana_tick = new_value;
	return 0;
}

static duk_ret_t js_add_stamina(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_stamina: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_stamina: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_stamina(new_value);
	return 0;
}

static duk_ret_t js_add_skill(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_skill: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_skill: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_skill(new_value);
	return 0;
}

static duk_ret_t js_add_luck(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_luck: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_luck: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_luck(new_value);
	return 0;
}

static duk_ret_t js_add_sneak(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_sneak: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_sneak: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_sneak(new_value);
	return 0;
}

static duk_ret_t js_add_speed(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_speed: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_speed: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_speed(new_value);
	return 0;
}

static duk_ret_t js_add_silver(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_silver: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_silver: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_silver(new_value);
	return 0;
}

static duk_ret_t js_add_gold(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_gold: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_gold: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_gold(new_value);
	return 0;
}

static duk_ret_t js_add_rations(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_rations: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_rations: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_rations(new_value);
	return 0;
}

static duk_ret_t js_add_experience(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_experience: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_experience: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_experience(new_value);
	return 0;
}

static duk_ret_t js_add_notoriety(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_notoriety: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_notoriety: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_notoriety(new_value);
	return 0;
}

static duk_ret_t js_add_mana(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS add_mana: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_number(cx, 0)) {
		bot->log(dpp::ll_warning, "JS add_mana: parameter 1 is not a number");
		return 0;
	}
	long new_value = (long)duk_get_number(cx, 0);
	paragraph& p = duk_get_udata(cx);
	p.cur_player->add_mana(new_value);
	return 0;
}

static duk_ret_t js_get_ach_key(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS get_ach_key: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	const char* key = duk_get_string(cx, 0);
	paragraph& p = duk_get_udata(cx);
	auto rs = db::query("SELECT achievements_kv FROM kv_store WHERE user_id = ? AND kv_key = ?", { p.cur_player->event.command.usr.id, key });
	if (rs.empty()) {
		duk_push_string(cx, "");
	} else {
		duk_push_string(cx, rs[0].at("kv_value").c_str());
	}
	return 1;
}

static duk_ret_t js_delete_ach_key(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS delete_ach_key: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	const char* key = duk_get_string(cx, 0);
	paragraph& p = duk_get_udata(cx);
	db::query("DELETE FROM achievements_kv WHERE user_id = ? AND kv_key = ?", { p.cur_player->event.command.usr.id, key });
	return 0;
}


static duk_ret_t js_set_ach_key(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 2) {
		bot->log(dpp::ll_warning, "JS set_ach_key: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	if (!duk_is_string(cx, 0)) {
		bot->log(dpp::ll_warning, "JS set_ach_key: parameter 1 is not a string");
		return 0;
	}
	std::string v;
	if (duk_is_string(cx, -1)) {
		const char* value = duk_get_string(cx, -1);
		v = value ? value : "";
	} else {
		auto value = (double)duk_get_number(cx, -1);
		v = fmt::format("{0:f}", value);
	}
	const char* key = duk_get_string(cx, 0);
	paragraph& p = duk_get_udata(cx);
	db::query("INSERT INTO achievements_kv (user_id, kv_key, kv_value) VALUES(?, ?, ?) ON DUPLICATE KEY UPDATE kv_value = ?", { p.cur_player->event.command.usr.id, key, v, v });
	return 0;
}

static duk_ret_t js_has_ach(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS has_ach: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	const char* slug = duk_get_string(cx, 0);
	paragraph& p = duk_get_udata(cx);
	auto rs = db::query("SELECT achievements.id FROM achievements JOIN achievements_unlocked ON achievements.id = achievements_unlocked.achievement_id AND user_id = ? WHERE slug = ?", { p.cur_player->event.command.usr.id, slug });
	duk_push_boolean(cx, !rs.empty());
	return 1;
}

static duk_ret_t js_unlock_ach(duk_context *cx) {
	int argc = duk_get_top(cx);
	if (argc != 1) {
		bot->log(dpp::ll_warning, "JS unlock_ach: incorrect number of parameters: " +std::to_string(argc));
		return 0;
	}
	const char* slug = duk_get_string(cx, 0);
	paragraph& p = duk_get_udata(cx);
	auto achievement = db::query("SELECT achievements.* FROM achievements WHERE slug = ?", { slug });
	if (!achievement.empty()) {
		db::query("INSERT INTO achievements_unlocked (user_id, achievement_id) VALUES(?, ?)", {p.cur_player->event.command.usr.id, achievement[0].at("id")});
		unlock_achievement(*(p.cur_player), achievement[0]);
	}
	return 0;
}



bool run(const std::string& script, paragraph& p, player& current_player, const std::map<std::string, json> &vars) {
	duk_int_t ret;

	duk_context* ctx = duk_create_heap(nullptr, nullptr, nullptr, (void*)&p, sandbox_fatal);

	duk_push_global_object(ctx);
	define_string(ctx, "BOT_ID", bot->me.id.str());
	define_number(ctx, "PARAGRAPH_ID", p.id);
	define_func(ctx, "print", js_print, DUK_VARARGS);
	define_func(ctx, "tag", js_tag, DUK_VARARGS);
	define_func(ctx, "exit", js_exit, 1);
	define_func(ctx, "get_name", js_get_name, 0);
	define_func(ctx, "get_race", js_get_race, 0);
	define_func(ctx, "get_profession", js_get_profession, 0);
	define_func(ctx, "get_gender", js_get_gender, 0);
	define_func(ctx, "get_stamina", js_get_stamina, 0);
	define_func(ctx, "get_skill", js_get_skill, 0);
	define_func(ctx, "get_luck", js_get_luck, 0);
	define_func(ctx, "get_sneak", js_get_sneak, 0);
	define_func(ctx, "get_speed", js_get_speed, 0);
	define_func(ctx, "get_silver", js_get_silver, 0);
	define_func(ctx, "get_gold", js_get_gold, 0);
	define_func(ctx, "get_rations", js_get_rations, 0);
	define_func(ctx, "get_experience", js_get_experience, 0);
	define_func(ctx, "get_level", js_get_level, 0);
	define_func(ctx, "get_notoriety", js_get_notoriety, 0);
	define_func(ctx, "get_days", js_get_days, 0);
	define_func(ctx, "get_scrolls", js_get_scrolls, 0);
	define_func(ctx, "get_paragraph", js_get_paragraph, 0);
	define_func(ctx, "get_armour", js_get_armour, 0);
	define_func(ctx, "get_weapon", js_get_weapon, 0);
	define_func(ctx, "get_last_use", js_get_last_use, 0);
	define_func(ctx, "get_last_strike", js_get_last_strike, 0);
	define_func(ctx, "get_pinned", js_get_pinned, 0);
	define_func(ctx, "get_muted", js_get_muted, 0);
	define_func(ctx, "get_mana", js_get_mana, 0);
	define_func(ctx, "get_mana_tick", js_get_mana_tick, 0);
	define_func(ctx, "set_name", js_set_name, 1);
	define_func(ctx, "set_race", js_set_race, 1);
	define_func(ctx, "set_profession", js_set_profession, 1);
	define_func(ctx, "set_gender", js_set_gender, 1);
	define_func(ctx, "set_stamina", js_set_stamina, 1);
	define_func(ctx, "set_skill", js_set_skill, 1);
	define_func(ctx, "set_luck", js_set_luck, 1);
	define_func(ctx, "set_sneak", js_set_sneak, 1);
	define_func(ctx, "set_speed", js_set_speed, 1);
	define_func(ctx, "set_silver", js_set_silver, 1);
	define_func(ctx, "set_gold", js_set_gold, 1);
	define_func(ctx, "set_rations", js_set_rations, 1);
	define_func(ctx, "set_experience", js_set_experience, 1);
	define_func(ctx, "set_notoriety", js_set_notoriety, 1);
	define_func(ctx, "set_days", js_set_days, 1);
	define_func(ctx, "set_scrolls", js_set_scrolls, 1);
	define_func(ctx, "set_paragraph", js_set_paragraph, 1);
	define_func(ctx, "set_armour", js_set_armour, 1);
	define_func(ctx, "set_weapon", js_set_weapon, 1);
	define_func(ctx, "set_last_use", js_set_last_use, 1);
	define_func(ctx, "set_last_strike", js_set_last_strike, 1);
	define_func(ctx, "set_pinned", js_set_pinned, 1);
	define_func(ctx, "set_muted", js_set_muted, 1);
	define_func(ctx, "set_mana", js_set_mana, 1);
	define_func(ctx, "set_mana_tick", js_set_mana_tick, 1);
	define_func(ctx, "add_stamina", js_add_stamina, 1);
	define_func(ctx, "add_skill", js_add_skill, 1);
	define_func(ctx, "add_luck", js_add_luck, 1);
	define_func(ctx, "add_sneak", js_add_sneak, 1);
	define_func(ctx, "add_speed", js_add_speed, 1);
	define_func(ctx, "add_silver", js_add_silver, 1);
	define_func(ctx, "add_gold", js_add_gold, 1);
	define_func(ctx, "add_rations", js_add_rations, 1);
	define_func(ctx, "add_experience", js_add_experience, 1);
	define_func(ctx, "add_notoriety", js_add_notoriety, 1);
	define_func(ctx, "add_mana", js_add_mana, 1);
	define_func(ctx, "set_key", js_set_key, 2);
	define_func(ctx, "get_key", js_get_key, 1);
	define_func(ctx, "delete_key", js_delete_key, 1);
	define_func(ctx, "toast", js_toast, 2);
	define_func(ctx, "set_ach_key", js_set_ach_key, 2);
	define_func(ctx, "get_ach_key", js_get_ach_key, 1);
	define_func(ctx, "delete_ach_key", js_delete_ach_key, 1);
	define_func(ctx, "has_ach", js_has_ach, 1);
	define_func(ctx, "unlock_ach", js_unlock_ach, 1);

	duk_pop(ctx);

	duk_push_string(ctx, "paragraph.js");
	std::string source;
	for (const auto & var : vars) {
		source += var.first + "=" + var.second.dump() + ";";
	}
	source += ";" + script;

	std::string lasterror;
	if (duk_pcompile_string_filename(ctx, 0, source.c_str()) != 0) {
		lasterror = duk_safe_to_string(ctx, -1);
		bot->log(dpp::ll_error, "JS error in script tag on paragraph " + std::to_string(p.id) + ": " + lasterror);
		duk_destroy_heap(ctx);
		return false;
	} else if (!duk_is_function(ctx, -1)) {
		bot->log(dpp::ll_error, "JS error in script tag on paragraph " + std::to_string(p.id) + ": Top of stack is not a function");
		duk_destroy_heap(ctx);
		return false;
	}

	ret = DUK_EXEC_SUCCESS;
	bool exited = false;
	try {
		ret = duk_pcall(ctx, 0);
	}
	catch (const exit_exception&) {
		/* Graceful exit from javascript via an exception for the exit() function */
		ret = DUK_EXEC_SUCCESS;
		exited = true;
	}

	if (ret != DUK_EXEC_SUCCESS) {
		if (duk_is_error(ctx, -1)) {
			duk_get_prop_string(ctx, -1, "stack");
			lasterror = duk_safe_to_string(ctx, -1);
			duk_pop(ctx);
		} else {
			lasterror = duk_safe_to_string(ctx, -1);
		}
		bot->log(dpp::ll_error, "JS error in script tag on paragraph " + std::to_string(p.id) + ": " + lasterror);
		duk_destroy_heap(ctx);
		return false;
	}
	if (!exited) {
		duk_pop(ctx);
	}
	duk_destroy_heap(ctx);
	return true;
}

void sandbox_fatal(void *udata, const char *msg) {
	// Yeah, according to the docs a fatal can never return. Technically, it doesnt.
	// At this point we should probably destroy the duk context as bad.
	throw std::runtime_error("Fatal JS error: " + std::string(msg));
}

}