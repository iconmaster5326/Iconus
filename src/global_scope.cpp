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
	using Arg = Function::Arg;
	
	globalScope.vars["echo"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"input", "", "",
			{}, {},
			[](auto session, auto scope, auto input, auto args, auto varargs, auto varflags) {
		return input;
			}
	));
	
	globalScope.vars["list"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "",
			{}, {},
			[](auto session, auto scope, auto input, auto args, auto varargs, auto varflags) {
		deque<Object*>* items = new deque<Object*>(varargs.begin(), varargs.end());
		return new Object(&ClassList::INSTANCE, items);
			}
	));
	
	globalScope.vars["apply"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "",
			{Arg("fn")}, {},
			[](auto session, auto scope, auto input, auto args, auto varargs, auto varflags) {
		return ((Object*)args["fn"])->execute(session, scope, input, varargs, varflags);
			}
	));
}
