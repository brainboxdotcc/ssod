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
#include <ssod/parser.h>
#include <ssod/database.h>

bool comparison(std::string condition, const std::string& lhs_str, const std::string& rhs_str, int g_dice) {
	condition = dpp::lowercase(condition);
	std::string rhs_actual = (rhs_str == "dice") ? std::to_string(g_dice) : rhs_str;
	long lhs_val, rhs_val;
	if (try_parse_long(lhs_str, lhs_val) && try_parse_long(rhs_actual, rhs_val)) {
		if (condition == "eq") return lhs_val == rhs_val;
		if (condition == "ne") return lhs_val != rhs_val;
		if (condition == "gt") return lhs_val > rhs_val;
		if (condition == "lt") return lhs_val < rhs_val;
		if (condition == "gte") return lhs_val >= rhs_val;
		if (condition == "lte") return lhs_val <= rhs_val;
	} else {
		if (condition == "eq") return lhs_str == rhs_actual;
		if (condition == "ne") return lhs_str != rhs_actual;
		if (condition == "gt") return lhs_str > rhs_actual;
		if (condition == "lt") return lhs_str < rhs_actual;
		if (condition == "gte") return lhs_str >= rhs_actual;
		if (condition == "lte") return lhs_str <= rhs_actual;
	}

	return false;
}

enum token_type {
	TOKEN_EOF,
	TOKEN_LPAREN,		// (
	TOKEN_RPAREN,		// )
	TOKEN_AND,		// and
	TOKEN_OR,		// or
	TOKEN_NOT,		// not
	TOKEN_IDENTIFIER,	// flag name quoted string, function
	TOKEN_COMPARISON,	// lt, gt, eq, ne, gte, lte
	TOKEN_NUMBER		// 0-9
};

struct token {
	token_type type{TOKEN_EOF};
	std::string value;
};

class tokenizer {
	std::istream& in;
public:
	explicit tokenizer(std::istream& stream) : in(stream) {}

	token next() {
		std::string val;
		char ch{};

		while (in.get(ch) && std::isspace(ch)) {}
		if (in.eof()) {
			return { TOKEN_EOF, "" };
		}

		switch (ch) {
			case '(': return { TOKEN_LPAREN, "(" };
			case ')': return { TOKEN_RPAREN, ")" };
		}

		val += ch;
		while (in.peek() != EOF && !std::isspace(in.peek()) && in.peek() != '(' && in.peek() != ')') {
			val += static_cast<char>(in.get());
		}

		std::string lowered = dpp::lowercase(val);
		if (lowered == "and") {
			return { TOKEN_AND, lowered };
		}
		if (lowered == "or") {
			return { TOKEN_OR, lowered };
		}
		if (lowered == "not") {
			return { TOKEN_NOT, lowered };
		}
		if (lowered == "eq" || lowered == "ne" || lowered == "gt" || lowered == "lt" || lowered == "gte" || lowered == "lte") {
			return { TOKEN_COMPARISON, lowered };
		}
		if (std::isdigit(val[0])) {
			return { TOKEN_NUMBER, lowered };
		}

		return { TOKEN_IDENTIFIER, val };
	}
};

class if_expression_parser {
	tokenizer lex;
	token current;
	player& current_player;
	int g_dice;

	void advance() {
		current = lex.next();
	}

	dpp::task<bool> parse_expression() {
		bool result = co_await parse_term();
		while (current.type == TOKEN_OR) {
			advance();
			bool right = co_await parse_term();
			result = result || right;
		}
		co_return result;
	}

	dpp::task<bool> parse_term() {
		bool result = co_await parse_factor();
		while (current.type == TOKEN_AND) {
			advance();
			bool right = co_await parse_factor();
			result = result && right;
		}
		co_return result;
	}

	dpp::task<bool> parse_factor() {
		if (current.type == TOKEN_NOT) {
			advance();
			bool result = co_await parse_factor();
			co_return !result;
		} else if (current.type == TOKEN_LPAREN) {
			advance();
			bool result = co_await parse_expression();
			if (current.type != TOKEN_RPAREN)
				throw std::runtime_error("Expected ')'");
			advance();
			co_return result;
		} else {
			co_return co_await parse_atom();
		}
	}

	dpp::task<bool> parse_atom() {
		if (current.type != TOKEN_IDENTIFIER)
			throw std::runtime_error("Expected identifier");

		std::string lhs = dpp::lowercase(current.value);
		advance();

		// Comparison path
		if (current.type == TOKEN_COMPARISON) {
			std::string op = current.value;
			advance();

			std::vector<std::string> rhs_tokens;
			// Accept any token type as RHS (except control tokens)
			while (current.type != TOKEN_EOF &&
			       current.type != TOKEN_AND &&
			       current.type != TOKEN_OR &&
			       current.type != TOKEN_RPAREN) {
				rhs_tokens.push_back(current.value);
				advance();
			}

			if (rhs_tokens.empty()) {
				throw std::runtime_error("Expected right-hand side after comparison operator");
			}

			std::string rhs = join_args(rhs_tokens);

			auto map = get_score_map(current_player);

			// Resolve lhs value from map if it exists
			std::string lhs_val = (map.find(lhs) != map.end()) ? std::to_string(map[lhs]) : lhs;

			// Resolve rhs from scoreboard if it's a stat name
			if (map.find(rhs) != map.end()) {
				rhs = std::to_string(map[rhs]);
			}

			co_return comparison(op, lhs_val, rhs, g_dice);
		}

		// Function-style predicate
		std::vector<std::string> args;
		while (current.type == TOKEN_IDENTIFIER || current.type == TOKEN_NUMBER || current.type == TOKEN_COMPARISON) {
			args.push_back(current.value);
			advance();
		}

		co_return co_await co_evaluate_function(lhs, args);
	}

	static std::string join_args(const std::vector<std::string>& input, size_t start_index = 0) {
		std::string joined;
		bool in_quotes = false;
		for (size_t i = start_index; i < input.size(); ++i) {
			const std::string& word = input[i];
			if (!word.empty() && word.front() == '\"') {
				in_quotes = true;
				joined += word.substr(1);
			} else if (!word.empty() && word.back() == '\"') {
				joined += " " + word.substr(0, word.size() - 1);
				break;
			} else if (in_quotes) {
				joined += " " + word;
			} else {
				joined += word;
				if (i + 1 < input.size()) joined += " ";
			}
		}
		return dpp::lowercase(joined);
	}

	dpp::task<bool> co_evaluate_function(const std::string& name, const std::vector<std::string>& args) {
		std::string joined = join_args(args);

		if (name == "item") {
			co_return current_player.has_herb(joined) || current_player.has_spell(joined) || current_player.has_possession(joined);
		}
		if (name == "!item") {
			co_return !current_player.has_herb(joined) && !current_player.has_spell(joined) && !current_player.has_possession(joined);
		}
		if (name == "has" && args.size() >= 2) {
			size_t n = std::stoul(args[0]);
			std::string item = join_args(args, 1);
			size_t qty = 0;
			for (const auto& i : current_player.possessions) {
				if (dpp::lowercase(i.name) == item) qty += i.qty;
			}
			co_return qty >= n;
		}
		if (name == "race" && !args.empty()) {
			std::string val = dpp::lowercase(args[0]);
			co_return (val == "human" && (current_player.race == race_human || current_player.race == race_barbarian)) ||
				  (val == "orc" && (current_player.race == race_orc || current_player.race == race_goblin)) ||
				  (val == "elf" && (current_player.race == race_elf || current_player.race == race_dark_elf)) ||
				  (val == "dwarf" && current_player.race == race_dwarf) ||
				  (val == "lesserorc" && current_player.race == race_lesser_orc);
		}
		if (name == "raceex" && !args.empty()) {
			std::string val = dpp::lowercase(args[0]);
			co_return (val == "dwarf" && current_player.race == race_dwarf) ||
				  (val == "human" && current_player.race == race_human) ||
				  (val == "orc" && current_player.race == race_orc) ||
				  (val == "elf" && current_player.race == race_elf) ||
				  (val == "barbarian" && current_player.race == race_barbarian) ||
				  (val == "goblin" && current_player.race == race_goblin) ||
				  (val == "darkelf" && current_player.race == race_dark_elf) ||
				  (val == "lesserorc" && current_player.race == race_lesser_orc);
		}
		if (name == "prof" && !args.empty()) {
			std::string val = dpp::lowercase(args[0]);
			co_return (val == "warrior" && (current_player.profession == prof_warrior || current_player.profession == prof_mercenary)) ||
				  (val == "wizard" && current_player.profession == prof_wizard) ||
				  (val == "thief" && (current_player.profession == prof_thief || current_player.profession == prof_assassin)) ||
				  (val == "woodsman" && current_player.profession == prof_woodsman);
		}
		if (name == "profex" && !args.empty()) {
			std::string val = dpp::lowercase(args[0]);
			co_return (val == "warrior" && current_player.profession == prof_warrior) ||
				  (val == "mercenary" && current_player.profession == prof_mercenary) ||
				  (val == "assassin" && current_player.profession == prof_assassin) ||
				  (val == "wizard" && current_player.profession == prof_wizard) ||
				  (val == "thief" && current_player.profession == prof_thief) ||
				  (val == "woodsman" && current_player.profession == prof_woodsman);
		}
		if (name == "mounted") {
			co_return current_player.has_flag("horse");
		}
		if ((name == "flag" || name == "!flag") && !args.empty()) {
			std::string flagname = args[0];
			std::string full_key = "gamestate_" + flagname + "%";
			db::paramlist p = { current_player.event.command.usr.id, full_key };
			auto rs = co_await db::co_query("SELECT kv_value FROM kv_store WHERE user_id = ? AND kv_key LIKE ?", p);
			std::string value = rs.empty() ? "" : rs[0].at("kv_value");
			if (args.size() >= 3) {
				std::string op = dpp::lowercase(args[1]);
				std::string rhs = args[2];

				auto map = get_score_map(current_player);
				if (map.find(rhs) != map.end()) {
					rhs = std::to_string(map[rhs]);
				}

				bool result = !rs.empty() && comparison(op, value, rhs, g_dice);
				co_return (name == "flag") ? result : !result;
			} else {
				// Legacy truthiness check
				bool global = co_await global_set(flagname);
				bool timed = co_await timed_set(current_player.event, flagname);
				bool result = !rs.empty() || global || timed;
				co_return (name == "flag") ? result : !result;
			}
		}
		if (name == "premium") {
			bool has_entitlement = !current_player.event.command.entitlements.empty();
			db::paramlist p = { current_player.event.command.usr.id };
			auto rs = co_await db::co_query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", p);
			bool result = has_entitlement || !rs.empty();
			co_return result;
		}
		if (name == "water") {
			co_return (current_player.has_spell("water") && current_player.has_component_herb("water")) ||
				  current_player.has_possession("water canister") ||
				  current_player.has_possession("water cannister"); // intentional misspelling for legacy content
		}

		co_return false;
	}

public:
	if_expression_parser(std::istream& stream, player& p)
		: lex(stream), current_player(p), g_dice(p.g_dice) {
		advance();
	}

	dpp::task<bool> parse() {
		bool result = co_await parse_expression();
		if (current.type != TOKEN_EOF)
			throw std::runtime_error("Unexpected input after expression");
		co_return result;
	}
};

struct if_tag : public tag {
	if_tag() { register_tag<if_tag>(); }
	static constexpr bool overrides_display{true};
	static constexpr std::string_view tags[]{"<if"};

	static dpp::task<void> route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string condition;
		paragraph_content >> p_text;

		if (p.display.empty() || p.display[p.display.size() - 1]) {
			try {
				std::string remaining;
				std::getline(paragraph_content, remaining, '>');
				std::stringstream ss(p_text + " " + remaining);
				if_expression_parser parser(ss, current_player);
				p.display.push_back(co_await parser.parse());
			} catch (const std::exception& e) {
				current_player.event.owner->log(dpp::ll_warning, "<IF> error on paragraph " + std::to_string(p.id) + ": " + std::string(e.what()));
				p.display.push_back(false);
			}
		} else {
			p.display.push_back(false);
		}
		co_return;
	}
};

static if_tag self_init;
