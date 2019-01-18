/*
 * session.cpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "classes.hpp"
#include "error.hpp"

#include <deque>

using namespace std;
using namespace iconus;

iconus::Session::Session() {
	globalScope.vars["echo"] = new Object(&ClassSystemFunction::INSTANCE, new ClassSystemFunction::Handler([]
	(Scope& scope, Object* input, const vector<Object*>& args, const unordered_map<string,Object*>& flags) {
		return input;
	}));
	
	globalScope.vars["list"] = new Object(&ClassSystemFunction::INSTANCE, new ClassSystemFunction::Handler([]
	(Scope& scope, Object* input, const vector<Object*>& args, const unordered_map<string,Object*>& flags) {
		deque<Object*>* items = new deque<Object*>(args.begin(), args.end());
		return new Object(&ClassList::INSTANCE, items);
	}));
}

std::string iconus::Session::evaluate(const std::string& input) {
	try {
		Lexer lexer(input);
		Op* op = parse(lexer);
		Object* result = op->evaluate(globalScope, &ClassNil::NIL);
		return result->operator string();
	} catch (const Error& e) {
		return "<div style=\"color:red\">error: " + string(e.what()) + "</div>";
	}
}
