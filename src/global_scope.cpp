/*
 * global_scope.cpp
 *
 *  Created on: Jan 18, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "classes.hpp"

#include <iostream>


using namespace std;
using namespace iconus;

void iconus::Session::addGlobalScope() {
	using Arg = Function::Arg;
	
	globalScope.vars["echo"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"input", "", "",
			{}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input;
			}
	));
	
	globalScope.vars["list"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "",
			{}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		Deque<Object*>* items = new Deque<Object*>(varargs.begin(), varargs.end());
		return new Object(&ClassList::INSTANCE, items);
			}
	));
	
	globalScope.vars["apply"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "flags",
			{Arg("fn")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return args["fn"]->execute(session, scope, input, varargs, varflags);
			}
	));
	
	globalScope.vars["get"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->getField(args["k"]);
			}
	));
	
	globalScope.vars["set"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k"), Arg("v")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		input->setField(args["k"], args["v"]);
		return input;
			}
	));
	
	globalScope.vars["local"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("v")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		string name = ClassString::value(args["v"]);
		scope.setLocal(name, input);
		return input;
			}
	));
	
	globalScope.vars["vars"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "", "",
			{}, {},
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		Deque<Object*> result;
		for (auto& pair : scope.vars) {
			result.push_back(ClassString::create(pair.first));
		}
		return ClassList::create(result);
			}
	));
}
