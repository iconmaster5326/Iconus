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

using namespace std;
using namespace iconus;

iconus::Session::Session() {
	globalScope.vars["echo"] = new Object(&ClassSystemFunction::INSTANCE, new ClassSystemFunction::Handler([](auto scope, auto input, auto args, auto flags) {
		return input;
	}));
}

std::string iconus::Session::evaluate(const std::string& input) {
	Lexer lexer(input);
	Op* op = parse(lexer);
	Object* result = op->evaluate(globalScope, nullptr);
	return result->operator string();
}
