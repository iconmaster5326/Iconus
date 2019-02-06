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

using Arg = Function::Arg;
constexpr Function::Role INPUT = Function::Role::INPUT;
constexpr Function::Role VARARG = Function::Role::VARARG;
constexpr Function::Role VARFLAG = Function::Role::VARFLAG;

extern "C" void iconus_initGlobalScope(GlobalScope& scope) {
	////////////////////////////
	// functions
	////////////////////////////
	
	scope.vars["echo"] = ClassManagedFunction::create(
			{Arg("i", INPUT)}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input;
			}
	);
	
	scope.vars["list"] = ClassManagedFunction::create(
			{Arg("args", VARARG)}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassList::create(varargs.begin(), varargs.end());
			}
	);
	
	scope.vars["apply"] = ClassManagedFunction::create(
			{Arg("fn"), Arg("args", VARARG)}, {Arg("flags", VARFLAG)},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return args["fn"]->execute(exe, scope, input, varargs, varflags);
			}
	);
	
	scope.vars["get"] = ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("k")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->getField(exe, args["k"]);
			}
	);
	
	scope.vars["set"] = ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("k"), Arg("v")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		input->setField(exe, args["k"], args["v"]);
		return input;
			}
	);
	
	scope.vars["local"] = ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("v")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		Lock lock{args["v"]->mutex};
		string name = ClassString::value(exe, args["v"]);
		scope.setLocal(name, input);
		return input;
			}
	);
	
	scope.vars["=="] = ClassManagedFunction::create(
			{Arg("a", INPUT), Arg("b")}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		Lock lockA{args["a"]->mutex};
		Lock lockB{args["b"]->mutex};
		return ClassBool::create(args["a"]->equals(args["b"]));
			}
	);
	
	scope.vars["get-class"] = ClassManagedFunction::create(
			{Arg("i", INPUT)}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassClass::create(input->clazz);
			}
	);
	
	scope.vars["to"] = ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("c")}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->adapt(exe, ClassClass::value(exe, args["c"]));
			}
	);
	
	scope.vars["is"] = ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("c")}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassBool::create(input->adaptableTo(exe, ClassClass::value(exe, args["c"])));
			}
	);
	
	scope.vars["map"] = ClassManagedFunction::create(
			{Arg("args", VARARG)}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		Object* result = ClassMap::create();
		auto& map = ClassMap::value(result);
		for (auto it = varargs.begin(); it != varargs.end(); it++) {
			Object* name = *it;
			it++;
			Object* value;
			if (it == varargs.end()) {
				value = &ClassNil::NIL;
			} else {
				value = *it;
			}
			
			map[name] = value;
			if (it == varargs.end()) break;
		}
		return result;
			}
	);
	
	scope.vars[">map"] = ClassManagedFunction::create(
			{Arg("i", INPUT)}, {},
			[](auto& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		Object* result = ClassMap::create();
		auto& map = ClassMap::value(result);
		for (auto& pair : input->fields(exe)) {
			map[pair.first] = pair.second;
		}
		return result;
			}
	);
	
	if (User::IS_ROOT) { // some commands, like login, are useless if we're not root
		scope.vars["login"] = ClassManagedFunction::create(
				{Arg("user"),Arg("pass")}, {},
				[](Execution& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
			Lock lockA{args["user"]->mutex};
			Lock lockB{args["pass"]->mutex};
			
			exe.session.user = User(ClassString::value(exe, args["user"]), ClassString::value(exe, args["pass"]));
			return &ClassNil::NIL;
				}
		);
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
	scope.vars["<map>"] = ClassClass::create(&ClassMap::INSTANCE);
	scope.vars["<table>"] = ClassClass::create(&ClassTable::INSTANCE);
	
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
	
	exe.session.renderers.emplace_back("table", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassTable::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		ostringstream sb;
		sb << "<table><tr>";
		
		Lock lock{ob->mutex};
		auto& table = ClassTable::value(exe, ob);
		for (Object* colName : table.colNames) {
			sb << "<th>" << exe.render(colName) << "</th>";
		}
		sb << "</tr>";
		
		for (Vector<Object*>& row : table.rows) {
			sb << "<tr>";
			for (Object* data : row) {
				sb << "<td>" << exe.render(data) << "</td>";
			}
			sb << "</tr>";
		}
		
		sb << "</table>";
		return sb.str();
	});
	
	exe.session.renderers.emplace_back("map as table", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassMap::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		ostringstream sb;
		sb << "<table>";
		
		Lock lock{ob->mutex};
		auto& map = ClassMap::value(exe, ob);
		for (auto& pair : map) {
			sb << "<tr><td>" << exe.render(pair.first) << "</td><td>" << exe.render(pair.second) << "</td></tr>";
		}
		
		sb << "</table>";
		return sb.str();
	});
	
	exe.session.renderers.emplace_back("ordered list", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassList::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		ostringstream sb;
		sb << "<ol>";
		
		Lock lock{ob->mutex};
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
		
		Lock lock{ob->mutex};
		Object* what = (Object*) ob->value.asPtr;
		sb << exe.render(what);
		
		sb << "</div>";
		return sb.str();
	});
	
	////////////////////////////
	// adaptors
	////////////////////////////
	
	if (exe.session.adaptors.find(&ClassString::INSTANCE) == exe.session.adaptors.end())
			exe.session.adaptors[&ClassString::INSTANCE] = {};
	exe.session.adaptors[&ClassNil::INSTANCE] = {};
	exe.session.adaptors[&ClassNumber::INSTANCE] = {};
	exe.session.adaptors[&ClassBool::INSTANCE] = {};
	exe.session.adaptors[&ClassClass::INSTANCE] = {};
	
	exe.session.adaptors[&ClassNil::INSTANCE][&ClassString::INSTANCE] = [](Execution& exe, Object* from) {
		return ClassString::create("");
	};
	
	exe.session.adaptors[&ClassBool::INSTANCE][&ClassString::INSTANCE] = [](Execution& exe, Object* from) {
		if (from == &ClassBool::TRUE) {
			return ClassString::create("true");
		} else if (from == &ClassBool::FALSE) {
			return ClassString::create("false");
		} else {
			throw runtime_error("bool wansn't TRUE or FALSE");
		}
	};
	
	exe.session.adaptors[&ClassClass::INSTANCE][&ClassString::INSTANCE] = [](Execution& exe, Object* from) {
		Class* value = ClassClass::value(exe, from);
		return ClassString::create("<class "+value->name()+">");
	};
	
	exe.session.adaptors[&ClassNumber::INSTANCE][&ClassString::INSTANCE] = [](Execution& exe, Object* from) {
		double value = ClassNumber::value(exe, from);
		string s = to_string(value);
		if (s.find('.') != string::npos) {
			while (s.back() == '0' || s.back() == '.') {
				char c = s.back();
				s.pop_back();
				if (c == '.') break;
			}
		}
		return ClassString::create(s.empty() ? "0" : s);
	};
	
	exe.session.adaptors[&ClassNumber::INSTANCE][&ClassBool::INSTANCE] = [](Execution& exe, Object* from) {
		double value = ClassNumber::value(exe, from);
		if (value == 0.0) return &ClassBool::FALSE; else return &ClassBool::TRUE;
	};
	
	exe.session.adaptors[&ClassString::INSTANCE][&ClassNumber::INSTANCE] = [](Execution& exe, Object* from) {
		try {
			Lock lock{from->mutex};
			const string& value = ClassString::value(exe, from);
			return ClassNumber::create(stod(value));
		} catch (const invalid_argument& e) {
			throw Error("Cannot adapt string to number: String is not a number");
		} catch (const out_of_range& e) {
			throw Error("Cannot adapt string to number: Number is out of range");
		}
	};
}
