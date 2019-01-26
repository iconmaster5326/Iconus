/*
 * plugin.cpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#include "classes.hpp"
#include "session.hpp"
#include "error.hpp"
#include "util.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace std;
using namespace iconus;

extern "C" string iconus_getName() {
	return "Standard Library";
}

extern "C" void iconus_initGlobalScope(GlobalScope& scope) {
	////////////////////////////
	// functions
	////////////////////////////
	
	using Arg = Function::Arg;
	
	scope.vars["echo"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"input", "", "",
			{}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input;
			}
	));
	
	scope.vars["list"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "",
			{}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassList::create(varargs.begin(), varargs.end());
			}
	));
	
	scope.vars["apply"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "flags",
			{Arg("fn")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return args["fn"]->execute(exe, scope, input, varargs, varflags);
			}
	));
	
	scope.vars["get"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->getField(exe, args["k"]);
			}
	));
	
	scope.vars["set"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k"), Arg("v")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		input->setField(exe, args["k"], args["v"]);
		return input;
			}
	));
	
	scope.vars["local"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("v")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		string name = ClassString::value(exe, args["v"]);
		scope.setLocal(name, input);
		return input;
			}
	));
	
	scope.vars["vars"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "", "",
			{}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		Deque<Object*> result;
		for (auto& pair : scope.vars) {
			result.push_back(ClassString::create(pair.first));
		}
		return ClassList::create(result);
			}
	));
	
	scope.vars["bool"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->adapt(exe, &ClassBool::INSTANCE);
			}
	));
	
	scope.vars["=="] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"a", "", "",
			{Arg("b")}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassBool::create(args["a"]->equals(args["b"]));
			}
	));
	
	scope.vars["get-class"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassClass::create(input->clazz);
			}
	));
	
	scope.vars["to"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("c")}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->adapt(exe, ClassClass::value(exe, args["c"]));
			}
	));
	
	scope.vars["is"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("c")}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassBool::create(input->adaptableTo(exe, ClassClass::value(exe, args["c"])));
			}
	));
	
	if (User::IS_ROOT) { // some commands, like login, are useless if we're not root
		scope.vars["login"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
				"", "", "",
				{Arg("user"),Arg("pass")}, {},
				[](Execution& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
			exe.session.user = User(ClassString::value(exe, args["user"]), ClassString::value(exe, args["pass"]));
			return &ClassNil::NIL;
				}
		));
	}
	
	////////////////////////////
	// constants
	////////////////////////////
	scope.vars["<nil>"] = ClassClass::create(&ClassNil::INSTANCE);
	scope.vars["<bool>"] = ClassClass::create(&ClassBool::INSTANCE);
	scope.vars["<number>"] = ClassClass::create(&ClassNumber::INSTANCE);
	scope.vars["<string>"] = ClassClass::create(&ClassString::INSTANCE);
	scope.vars["<list>"] = ClassClass::create(&ClassList::INSTANCE);
	scope.vars["<error>"] = ClassClass::create(&ClassError::INSTANCE);
	scope.vars["<class>"] = ClassClass::create(&ClassClass::INSTANCE);
	
	scope.vars["PI"] = ClassNumber::create(3.14159265358979323846);
	scope.vars["E"] = ClassNumber::create(2.71828182845904523536);
}

extern "C" void iconus_initSession(Execution& exe) {
	////////////////////////////
	// word parsers
	////////////////////////////
	
	exe.session.parsers.emplace_back([](Execution& exe, const std::string& word) {
		return word == "nil";
	},[](Execution& exe, string word) {
		return &ClassNil::NIL;
	});
	
	exe.session.parsers.emplace_back([](Execution& exe, const std::string& word) {
		return word == "true";
	},[](Execution& exe, string word) {
		return &ClassBool::TRUE;
	});
	
	exe.session.parsers.emplace_back([](Execution& exe, const std::string& word) {
		return word == "false";
	},[](Execution& exe, string word) {
		return &ClassBool::FALSE;
	});
	
	exe.session.parsers.emplace_back([](Execution& exe, const std::string& word) {
		try {
			stod(word);
			return true;
		} catch (const invalid_argument& ex) {
			return false;
		} catch (const out_of_range& ex) {
			throw Error("Numeric constant out of range: "+word);
		}
	},[](Execution& exe, string word) {
		return new Object(&ClassNumber::INSTANCE, stod(word));
	});
	
	////////////////////////////
	// renderers
	////////////////////////////
	
	exe.session.renderers.emplace_back("numbered list", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassList::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		ostringstream sb;
		sb << "<ol>";
		
		Deque<Object*>& items = *((Deque<Object*>*)ob->value.asPtr);
		for (Object* item : items) {
			sb << "<li>" << exe.render(item) << "</li>";
		}
		
		sb << "</ol>";
		return sb.str();
	});
	
	exe.session.renderers.emplace_back("error", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassError::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		ostringstream sb;
		sb << "<div style=\"color: red;\"><b>error:</b> ";
		
		Object* what = (Object*) ob->value.asPtr;
		sb << exe.render(what);
		
		sb << "</div>";
		return sb.str();
	});
	
	////////////////////////////
	// adaptors
	////////////////////////////
	
	exe.session.adaptors[&ClassNumber::INSTANCE] = {};
	exe.session.adaptors[&ClassString::INSTANCE] = {};
	exe.session.adaptors[&ClassBool::INSTANCE] = {};
	
	exe.session.adaptors[&ClassNumber::INSTANCE][&ClassString::INSTANCE] = [](Execution& exe, Object* from) {
		double value = ClassNumber::value(exe, from);
		return ClassString::create(to_string(value));
	};
	
	exe.session.adaptors[&ClassNumber::INSTANCE][&ClassBool::INSTANCE] = [](Execution& exe, Object* from) {
		double value = ClassNumber::value(exe, from);
		if (value == 0.0) return &ClassBool::FALSE; else return &ClassBool::TRUE;
	};
	
	exe.session.adaptors[&ClassString::INSTANCE][&ClassNumber::INSTANCE] = [](Execution& exe, Object* from) {
		try {
			const string& value = ClassString::value(exe, from);
			return ClassNumber::create(stod(value));
		} catch (const invalid_argument& e) {
			throw Error("Cannot adapt string to number: String is not a number");
		} catch (const out_of_range& e) {
			throw Error("Cannot adapt string to number: Number is out of range");
		}
	};
}
