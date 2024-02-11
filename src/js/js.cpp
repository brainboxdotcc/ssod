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
#include <ssod/config.h>
#include <ssod/database.h>
#include <thread>
#include <streambuf>
#include <fstream>
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "duktape.h"
#include <ssod/paragraph.h>
#include <ssod/parser.h>

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
	for (auto i = strings.begin(); i != strings.end(); ++i) {
		duk_push_string(cx, i->first.c_str());
		duk_push_string(cx, i->second.c_str());
		duk_put_prop(cx, obj_idx);
	}
	for (auto i = bools.begin(); i != bools.end(); ++i) {
		duk_push_string(cx, i->first.c_str());
		duk_push_boolean(cx, i->second);
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

bool run(const std::string& script, paragraph& p, player& current_player, const std::map<std::string, json> &vars) {
	duk_int_t ret;

	duk_context* ctx = duk_create_heap(nullptr, nullptr, nullptr, (void*)&p, sandbox_fatal);
	duk_push_global_object(ctx);
	define_string(ctx, "BOT_ID", bot->me.id.str());
	define_number(ctx, "PARAGRAPH_ID", p.id);
	define_func(ctx, "print", js_print, DUK_VARARGS);
	define_func(ctx, "tag", js_tag, DUK_VARARGS);
	define_func(ctx, "exit", js_exit, 1);
	duk_pop(ctx);

	duk_push_string(ctx, "paragraph.js");
	std::string source;
	for (auto i = vars.begin(); i != vars.end(); ++i) {
		source += i->first + "=" + i->second.dump() + ";";
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