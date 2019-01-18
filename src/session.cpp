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
	addDefaultRenderers();
	addGlobalScope();
}

Object* iconus::Session::evaluate(const std::string& input) {
	Lexer lexer(input);
	Op* op = parse(*this, lexer);
	try {
		return op->evaluate(*this, globalScope, &ClassNil::NIL);
	} catch (const Error& e) {
		return e.value;
	}
}

std::string iconus::Session::render(Object* ob) {
	for (const Renderer& renderer : renderers) {
		if (renderer.filter(*this, ob)) {
			return renderer.handler(*this, ob);
		}
	}
	
	throw runtime_error("no renderer defined for object!");
}
