/*
 * global_scope.cpp
 *
 *  Created on: Jan 18, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "classes.hpp"

using namespace std;
using namespace iconus;

void iconus::Session::addGlobalScope() {
	globalScope.vars["echo"] = new Object(&ClassSystemFunction::INSTANCE, new ClassSystemFunction::Handler([]
	(Session& session, Scope& scope, Object* input, const vector<Object*>& args, const unordered_map<string,Object*>& flags) {
		return input;
	}));
	
	globalScope.vars["list"] = new Object(&ClassSystemFunction::INSTANCE, new ClassSystemFunction::Handler([]
	(Session& session, Scope& scope, Object* input, const vector<Object*>& args, const unordered_map<string,Object*>& flags) {
		deque<Object*>* items = new deque<Object*>(args.begin(), args.end());
		return new Object(&ClassList::INSTANCE, items);
	}));
}
