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
	globalScope.vars["echo"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"in", "", "",
			{}, {},
			[](auto session, auto scope, auto input, auto args, auto varargs, auto varflags) {
		return input;
			}
	));
	
	globalScope.vars["list"] = new Object(&ClassSystemFunction::INSTANCE, new ClassSystemFunction::Handler([]
	(auto session, auto scope, auto input, auto args, auto flags) {
		deque<Object*>* items = new deque<Object*>(args.begin(), args.end());
		return new Object(&ClassList::INSTANCE, items);
	}));
}
