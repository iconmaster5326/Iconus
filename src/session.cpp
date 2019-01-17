/*
 * session.cpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "lexer.hpp"
#include "parser.hpp"

using namespace std;
using namespace iconus;

iconus::Session::Session() {
	// TODO
}

std::string iconus::Session::evaluate(const std::string& input) {
	Lexer lexer(input);
	Op* op = parse(lexer);
	Object* result = op->evaluate(globalScope, nullptr);
	return result->operator string();
}
