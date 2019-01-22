/*
 * plugin.cpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "classes.hpp"
#include "error.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace iconus;
using namespace boost::filesystem;

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
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input;
			}
	));
	
	scope.vars["list"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "",
			{}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassList::create(varargs.begin(), varargs.end());
			}
	));
	
	scope.vars["apply"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "flags",
			{Arg("fn")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return args["fn"]->execute(session, scope, input, varargs, varflags);
			}
	));
	
	scope.vars["get"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->getField(session, args["k"]);
			}
	));
	
	scope.vars["set"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k"), Arg("v")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		input->setField(session, args["k"], args["v"]);
		return input;
			}
	));
	
	scope.vars["local"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("v")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		string name = ClassString::value(session, args["v"]);
		scope.setLocal(name, input);
		return input;
			}
	));
	
	scope.vars["vars"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
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
	
	scope.vars["bool"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{}, {},
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->adapt(session, &ClassBool::INSTANCE);
			}
	));
	
	scope.vars["=="] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"a", "", "",
			{Arg("b")}, {},
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassBool::create(args["a"]->equals(args["b"]));
			}
	));
	
	scope.vars["get-class"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{}, {},
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassClass::create(input->clazz);
			}
	));
	
	scope.vars["ls"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"dir", "", "",
			{}, {},
			[](Session& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		try {
			Object* result = ClassList::create();
			Deque<Object*>& items = ClassList::value(session, result);
			session.user.doAsUser([&]() {
				path p{input == &ClassNil::NIL ? "." : ClassString::value(session, input)};
				for (directory_iterator it{p}; it != directory_iterator{}; it++) {
					items.push_back(ClassString::create(it->path().string()));
				}
			});
			return result;
		} catch (const filesystem_error& e) {
			throw Error(e.what());
		}
			}
	));
	
	scope.vars["login"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "", "",
			{Arg("user"),Arg("pass")}, {},
			[](Session& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		session.user = User(ClassString::value(session, args["user"]), ClassString::value(session, args["pass"]));
		return &ClassNil::NIL;
			}
	));
}

extern "C" void iconus_initSession(Session& session) {
	////////////////////////////
	// word parsers
	////////////////////////////
	
	session.parsers.emplace_back([](Session& session, const std::string& word) {
		return word == "nil";
	},[](Session& session, string word) {
		return &ClassNil::NIL;
	});
	
	session.parsers.emplace_back([](Session& session, const std::string& word) {
		return word == "true";
	},[](Session& session, string word) {
		return &ClassBool::TRUE;
	});
	
	session.parsers.emplace_back([](Session& session, const std::string& word) {
		return word == "false";
	},[](Session& session, string word) {
		return &ClassBool::FALSE;
	});
	
	session.parsers.emplace_back([](Session& session, const std::string& word) {
		try {
			stod(word);
			return true;
		} catch (const invalid_argument& ex) {
			return false;
		} catch (const out_of_range& ex) {
			throw Error("Numeric constant out of range: "+word);
		}
	},[](Session& session, string word) {
		return new Object(&ClassNumber::INSTANCE, stod(word));
	});
	
	////////////////////////////
	// renderers
	////////////////////////////
	
	session.renderers.emplace_back("numbered list", [](Session& session, Object* ob) {
		return ob->clazz == &ClassList::INSTANCE;
	}, [](Session& session, Object* ob) {
		ostringstream sb;
		sb << "<ol>";
		
		Deque<Object*>& items = *((Deque<Object*>*)ob->value.asPtr);
		for (Object* item : items) {
			sb << "<li>" << session.render(item) << "</li>";
		}
		
		sb << "</ol>";
		return sb.str();
	});
	
	session.renderers.emplace_back("error", [](Session& session, Object* ob) {
		return ob->clazz == &ClassError::INSTANCE;
	}, [](Session& session, Object* ob) {
		ostringstream sb;
		sb << "<div style=\"color: red;\"><b>error:</b> ";
		
		Object* what = (Object*) ob->value.asPtr;
		sb << session.render(what);
		
		sb << "</div>";
		return sb.str();
	});
	
	session.renderers.emplace_back("raw string", [](Session& session, Object* ob) {
		return true;
	}, [](Session& session, Object* ob) {
		return ob->toString(session);
	});
	
	////////////////////////////
	// adaptors
	////////////////////////////
	
	session.adaptors[&ClassNumber::INSTANCE] = {};
	session.adaptors[&ClassString::INSTANCE] = {};
	session.adaptors[&ClassBool::INSTANCE] = {};
	
	session.adaptors[&ClassNumber::INSTANCE][&ClassString::INSTANCE] = [](Session& session, Object* from) {
		double value = ClassNumber::value(session, from);
		return ClassString::create(to_string(value));
	};
	
	session.adaptors[&ClassNumber::INSTANCE][&ClassBool::INSTANCE] = [](Session& session, Object* from) {
		double value = ClassNumber::value(session, from);
		if (value == 0.0) return &ClassBool::FALSE; else return &ClassBool::TRUE;
	};
	
	session.adaptors[&ClassString::INSTANCE][&ClassNumber::INSTANCE] = [](Session& session, Object* from) {
		const string& value = ClassString::value(session, from);
		return ClassNumber::create(stod(value));
	};
}
