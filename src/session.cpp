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



using namespace std;
using namespace iconus;

iconus::Session::Session() {
	addDefaultRenderers();
	addDefaultWordParsers();
	addGlobalScope();
}

Object* iconus::Session::evaluate(const std::string& input) {
	try {
		Lexer lexer(input);
		Op* op = parse(*this, lexer);
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

Object* iconus::Session::parseWord(std::string word) {
	for (const WordParser& parser : parsers) {
		if (parser.filter(*this, word)) {
			return parser.handler(*this, word);
		}
	}
	
	return nullptr;
}
