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

iconus::Session::Session() : sessionScope(&GlobalScope::INSTANCE) {
	addDefaultRenderers();
	addDefaultWordParsers();
	addDefaultAdaptors();
}

Object* iconus::Session::evaluate(const std::string& input) {
	try {
		Lexer lexer(input);
		Op* op = parse(*this, lexer);
		return op->evaluate(*this, sessionScope, &ClassNil::NIL);
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

static Adaptor getAdaptor(Session& session, Class* from, Class* to, Set<Class*>& checked) {
	if (from == to) return [](auto& session, auto ob) {
		return ob;
	};
	if (checked.find(from) != checked.end()) return nullptr;
	checked.insert(from);
	
	auto it = session.adaptors.find(from);
	if (it == session.adaptors.end()) {
		return nullptr;
	} else {
		Map<Class*, Adaptor>& adaptors = it->second;
		auto it = adaptors.find(to);
		if (it == adaptors.end()) {
			for (auto& pair : adaptors) {
				Adaptor adaptor1 = pair.second;
				Adaptor adaptor2 = getAdaptor(session, pair.first, to, checked);
				
				if (adaptor2) {
					return [adaptor1,adaptor2](auto& session, auto ob) {
						return adaptor2(session, adaptor1(session, ob));
					};
				}
			}
			
			return nullptr;
		} else {
			return it->second;
		}
	}
}

Adaptor iconus::Session::getAdaptor(Class* from, Class* to) {
	Set<Class*> checked;
	return ::getAdaptor(*this, from, to, checked);
}

GlobalScope GlobalScope::INSTANCE{};

bool iconus::GlobalScope::canSet(const std::string& name) {
	return false;
}
