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
#include "plugin.hpp"
#include "util.hpp"

#include <iostream>

using namespace std;
using namespace iconus;

iconus::Session::Session() : sessionScope(&GlobalScope::INSTANCE), closed(false) {
	for (Plugin& p : Plugin::plugins) {
		try {
			p.initSession(*this);
		} catch (const exception& e) {
			cout << "WARNING: error in plugin initSession: " << e.what() << endl;
		}
	}
}

Object* iconus::Session::evaluate(const std::string& input, Execution& exe) {
	static const string LOCATION{"input"};
	
	try {
		Lexer lexer(LOCATION, input);
		Op* op = parse(exe, lexer);
		return op->evaluate(exe, sessionScope, &ClassNil::NIL);
	} catch (const Error& e) {
		return ClassError::create(e);
	}
}

std::string iconus::Execution::render(Object* ob) {
	for (const Renderer& renderer : session.renderers) {
		if (renderer.filter(*this, ob)) {
			return renderer.handler(*this, ob);
		}
	}
	
	try {
		return escapeHTML(ob->toString(*this));
	} catch (const Error& e) {
		ostringstream sb;
		sb << '<' << ob->clazz->name() << ' ' << ((void*)ob) << '>';
		return escapeHTML(sb.str());
	}
}

Object* iconus::Execution::parseWord(std::string word) {
	for (const WordParser& parser : session.parsers) {
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

Adaptor iconus::Execution::getAdaptor(Class* from, Class* to) {
	Set<Class*> checked;
	return ::getAdaptor(session, from, to, checked);
}

static int adaptionDistance(Session& session, Class* from, Class* to, Set<Class*>& checked) {
	if (from == to) return 0;
	if (checked.find(from) != checked.end()) return -1;
	checked.insert(from);
	
	auto it = session.adaptors.find(from);
	if (it == session.adaptors.end()) {
		return -1;
	} else {
		Map<Class*, Adaptor>& adaptors = it->second;
		auto it = adaptors.find(to);
		if (it == adaptors.end()) {
			for (auto& pair : adaptors) {
				int d = adaptionDistance(session, pair.first, to, checked);
				if (d == -1)
					return -1;
				else
					return d+1;
			}
			
			return -1;
		} else {
			return 1;
		}
	}
}

int iconus::Execution::adaptionDistance(Class* from, Class* to) {
	Set<Class*> checked;
	return ::adaptionDistance(session, from, to, checked);
}

GlobalScope GlobalScope::INSTANCE{};

bool iconus::GlobalScope::canSet(const std::string& name) {
	return false;
}

Object* iconus::Execution::cat(const std::string& file) {
	for (const CatHandler& catter : session.catHandlers) {
		if (catter.filter(*this, file)) {
			return catter.handler(*this, file);
		}
	}
	
	throw runtime_error("no catter defined for file!");
}

void iconus::Session::addHistory(const string& input, Object* output) {
	if (maxHistory == 0) {
		history.clear();
		return;
	}
	
	if (history.size() >= maxHistory) {
		history.erase(history.begin(), history.end()-maxHistory+1);
	}
	
	history.emplace_back(input, output);
}
