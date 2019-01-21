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

iconus::GlobalScope::GlobalScope() : Scope() {
	using Arg = Function::Arg;
	
	vars["echo"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"input", "", "",
			{}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input;
			}
	));
	
	vars["list"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "",
			{}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassList::create(varargs.begin(), varargs.end());
			}
	));
	
	vars["apply"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "flags",
			{Arg("fn")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return args["fn"]->execute(session, scope, input, varargs, varflags);
			}
	));
	
	vars["get"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->getField(session, args["k"]);
			}
	));
	
	vars["set"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k"), Arg("v")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		input->setField(session, args["k"], args["v"]);
		return input;
			}
	));
	
	vars["local"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("v")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		string name = ClassString::value(session, args["v"]);
		scope.setLocal(name, input);
		return input;
			}
	));
	
	vars["vars"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
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
	
	vars["bool"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{}, {},
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->adapt(session, &ClassBool::INSTANCE);
			}
	));
	
	vars["=="] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"a", "", "",
			{Arg("b")}, {},
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassBool::create(args["a"]->equals(args["b"]));
			}
	));
	
	vars["get-class"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{}, {},
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassClass::create(input->clazz);
			}
	));
}
