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
#include "parser.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include <thread>

using namespace std;
using namespace iconus;
using namespace boost::posix_time;

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
		StackTrace::enter(StackTrace::Type::FUNCTION);
		Object* result = args["fn"]->execute(exe, scope, input, varargs, varflags);
		StackTrace::exit();
		return result;
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
	
	scope.vars["select"] = ClassManagedFunction::create(
			{Arg("cols", VARARG)}, {},
			[](auto& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		Lock lock{input->mutex};
		ClassTable::Instance& inputTable = ClassTable::value(exe, input);
		
		Object* result = ClassTable::create();
		ClassTable::Instance& outputTable = ClassTable::value(result);
		
		Map<int,int> colMapping;
		int i = 0;
		for (Object* col : varargs) {
			outputTable.colNames.push_back(col);
			
			auto it = find_if(inputTable.colNames.begin(), inputTable.colNames.end(), [col](Object* col2) {
				return col->equals(col2);
			});
			
			if (it == inputTable.colNames.end()) {
				throw Error("select: Column name does not appear in input table: "+col->toString(exe));
			} else {
				colMapping[i] = it - inputTable.colNames.begin();
			}
			
			i++;
		}
		
		for (Vector<Object*>& inRow : inputTable.rows) {
			outputTable.rows.emplace_back();
			Vector<Object*>& outRow = outputTable.rows.back();
			
			for (int i = 0; i < outputTable.colNames.size(); i++) {
				outRow.push_back(inRow[colMapping[i]]);
			}
		}
		
		return result;
			}
	);
	
	scope.vars["ans"] = ClassManagedFunction::create(
			{Arg("n", ClassNumber::create(1.0))}, {},
			[](auto& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		int n = (int) ClassNumber::value(exe, args["n"]);
		if (n <= 0 || n > exe.session.history.size()) {
			throw Error("ans: Argument outside of history range: "+to_string(n));
		}
		
		return exe.session.history[exe.session.history.size()-n].output;
			}
	);
	
	scope.addMethod("filter", &ClassList::INSTANCE, ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("fn")}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		return ClassClass::create(input->clazz);
			}), ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("fn")}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		Lock lock{input->mutex};
		
		Object* result = ClassList::create();
		auto& list = ClassList::value(result);
		
		for (Object* val : input->fieldValues(exe)) {
			Vector<Object*> predArgs;
			Map<string,Object*> predFlags;
			
			StackTrace::enter(StackTrace::Type::FUNCTION);
			Object* predicate = args["fn"]->execute(exe, scope, val, predArgs, predFlags);
			StackTrace::exit();
			
			if (predicate->truthy()) {
				list.push_back(val);
			}
		}
		
		return result;
			}
	));
	
	scope.addMethod("filter", &ClassMap::INSTANCE, ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("fn")}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		return ClassClass::create(input->clazz);
			}), ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("fn")}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		Lock lock{input->mutex};
		
		Object* result = ClassMap::create();
		auto& map = ClassMap::value(result);
		
		for (auto& pair : input->fields(exe)) {
			Vector<Object*> predArgs;
			Map<string,Object*> predFlags;
			
			StackTrace::enter(StackTrace::Type::FUNCTION);
			Object* predicate = args["fn"]->execute(exe, scope, pair.second, predArgs, predFlags);
			StackTrace::exit();
			
			if (predicate->truthy()) {
				map[pair.first] = pair.second;
			}
		}
		
		return result;
			}
	));
	
	scope.addMethod("filter", &ClassTable::INSTANCE, ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("fn")}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		return ClassClass::create(input->clazz);
			}), ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("fn")}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		Lock lock{input->mutex};
		auto& oldTable = ClassTable::value(exe, input);
		
		Object* result = ClassTable::create();
		auto& newTable = ClassTable::value(result);
		
		newTable.colNames = oldTable.colNames;
		
		for (auto& row : oldTable.rows) {
			Vector<Object*> predArgs;
			Map<string,Object*> predFlags;
			
			Object* newInput = ClassMap::create();
			auto& map = ClassMap::value(newInput);
			int i = 0;
			for (Object* name : oldTable.colNames) {
				map[name] = row[i];
				i++;
			}
			
			StackTrace::enter(StackTrace::Type::FUNCTION);
			Object* predicate = args["fn"]->execute(exe, scope, newInput, predArgs, predFlags);
			StackTrace::exit();
			
			if (predicate->truthy()) {
				newTable.rows.push_back(row);
			}
		}
		
		return result;
			}
	));
	
	scope.vars["method"] = ClassManagedFunction::create(
			{Arg("default", &ClassNil::NIL)}, {},
			[](Execution& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		Object* result = ClassMethod::create();
		ClassMethod::Instance& method = ClassMethod::value(result);
		method.mutableByUser = true;
		if (args["default"] != &ClassNil::NIL) {
			method.defaultHandler = args["default"];
		}
		return result;
			}
	);
	
	scope.vars["override!"] = ClassManagedFunction::create(
			{Arg("i", INPUT), Arg("class"), Arg("selector", &ClassNil::NIL), Arg("handler")}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		Lock lock{input->mutex};
		ClassMethod::Instance& method = ClassMethod::value(input);
		if (!method.mutableByUser)
			throw Error("override!: Cannot override immutable method");
		
		if (args["selector"] == &ClassNil::NIL) {
			args["selector"] = ClassSystemFunction::create([](Execution& exe, Scope& scope, auto input, auto& args, auto& flags) {
				return ClassClass::create(input->clazz);
			});
		}
		Class* clazz = ClassClass::value(exe, args["class"]);;
		
		method.handlers.emplace_back(clazz, args["selector"], args["handler"]);
		
		return input;
			}
	);
	
	scope.vars["eval"] = ClassManagedFunction::create(
			{Arg("s")}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		static const string LOCATION = "eval";
		
		Op* op; {
			Lock lock{args["s"]->mutex};
			string& s = ClassString::value(exe, args["s"]);
			Lexer lexer{LOCATION, s};
			op = parse(exe, lexer);
		}
		
		return op->evaluate(exe, scope, input);
			}
	);
	
	scope.addMethod(">table", &ClassList::INSTANCE, ClassManagedFunction::create(
			{Arg("i", INPUT)}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		return ClassClass::create(input->clazz);
			}), ClassManagedFunction::create(
			{Arg("i", INPUT)}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		Object* result = ClassTable::create();
		auto& newTable = ClassTable::value(result);
		
		for (auto& map : input->fieldValues(exe)) {
			newTable.rows.emplace_back(newTable.colNames.size(), &ClassNil::NIL);
			Vector<Object*>& newRow = newTable.rows.back();
			
			for (auto& pair : map->fields(exe)) {
				auto it = find_if(newTable.colNames.begin(), newTable.colNames.end(), [&](Object* colName) {
					return colName->equals(pair.first);
				});
				if (it == newTable.colNames.end()) {
					newTable.colNames.push_back(pair.first);
					for (Vector<Object*>& row : newTable.rows) {
						row.push_back(&ClassNil::NIL);
					}
					newRow.back() = pair.second;
				} else {
					ptrdiff_t i = it - newTable.colNames.begin();
					newRow[i] = pair.second;
				}
			}
		}
		
		return result;
			}
	));
	
	scope.vars["class"] = ClassManagedFunction::create(
			{Arg("name"), Arg("fields")}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		ClassUserDefined* newClass = new ClassUserDefined();
		newClass->className = ClassString::value(exe, args["name"]);
		auto& fs = ClassList::value(exe, args["fields"]);
		newClass->classFields.insert(newClass->classFields.begin(), fs.begin(), fs.end());
		return ClassClass::create(newClass);
			}
	);
	
	scope.vars["event"] = ClassManagedFunction::create(
			{}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		return ClassEvent::create(ClassList::create());
			}
	);
	
	scope.vars["connect"] = ClassManagedFunction::create(
			{Arg("event", INPUT), Arg("handler")}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		return ClassEvent::connect(input->adapt(exe, &ClassEvent::INSTANCE), args["handler"]);
			}
	);
	
	scope.vars["disconnect"] = ClassManagedFunction::create(
			{Arg("conn", INPUT)}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		ClassEventConnection::disconnect(input->adapt(exe, &ClassEventConnection::INSTANCE));
		return &ClassNil::NIL;
			}
	);
	
	scope.vars["fire"] = ClassManagedFunction::create(
			{Arg("args", VARARG)}, {Arg("flags", VARFLAG)},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		ClassEvent::fire(input->adapt(exe, &ClassEvent::INSTANCE), exe, scope, input, varargs, varflags);
		return &ClassNil::NIL;
			}
	);
	
	scope.vars["spawn"] = ClassManagedFunction::create(
			{Arg("fns", VARARG)}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
		for (Object* arg : varargs) {
			thread t{[&exe, &scope, input, arg]() {
				Vector<Object*> args; Map<string,Object*> flags;
				
				try {
					arg->execute(exe, scope, input, args, flags);
				} catch (const Error& e) {
					// TODO: log the error?
				}
			}};
			t.detach();
		}
		return &ClassNil::NIL;
			}
	);
	
	scope.vars["wait"] = ClassManagedFunction::create(
			{Arg("event", INPUT), Arg("handler", &ClassNil::NIL)}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& margs, auto& varargs, auto& varflags) {
		Object* result = nullptr;
		Object* conn = ClassEvent::connect(input->adapt(exe, &ClassEvent::INSTANCE), ClassSystemFunction::create(
		[&](Execution& exe, Scope& scope, Object* input, auto& args, auto& flags) {
			if (margs["handler"]->truthy()) {
				result = margs["handler"]->execute(exe, scope, input, args, flags);
			} else {
				result = input;
			}
			return result;
		}));
		
		while (!result) this_thread::yield();
		ClassEventConnection::disconnect(conn);
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
	scope.vars["<num>"] = ClassClass::create(&ClassNumber::INSTANCE);
	scope.vars["<str>"] = ClassClass::create(&ClassString::INSTANCE);
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
		Error& error = ClassError::value(exe, ob);
		sb << exe.render(error.value);
		
		for (auto it = error.stackTrace.rbegin(); it != error.stackTrace.rend(); it++) {
			StackTrace& stackTrace = *it;
			sb << "<p class=\"stack-trace\">";
			
			switch (stackTrace.type) {
			case (StackTrace::Type::SYNTAX): {
				sb << "at parser";
			} break;
			case (StackTrace::Type::FUNCTION): {
				if (stackTrace.name.empty()) {
					sb << "at anonymous function";
				} else {
					sb << "at function " << stackTrace.name;
				}
			} break;
			}
			
			if (stackTrace.source.location) {
				sb << " (" << *(stackTrace.source.location);
				if (stackTrace.source.line != -1) {
					sb << " line " << stackTrace.source.line;
				}
				if (stackTrace.source.col != -1) {
					sb << " col " << stackTrace.source.col;
				}
				sb << ")";
			}
			
			sb << "</p>";
		}
		
		sb << "</div>";
		return sb.str();
	});
	
	exe.session.renderers.emplace_back("user defined renderer", [](Execution& exe, Object* ob) {
		Object* methodName = ClassString::create("__render__");
		return ob->clazz->hasStaticField(exe, methodName) && ob->clazz->getStaticField(exe, methodName)->truthy();
	}, [](Execution& exe, Object* ob) {
		Object* methodName = ClassString::create("__render__");
		Object* method = ob->clazz->getStaticField(exe, methodName);
		Vector<Object*> args; Map<string,Object*> flags;
		Object* result = method->execute(exe, exe.session.sessionScope, ob, args, flags);
		return exe.render(result);
	});
	
	exe.session.renderers.emplace_back("map*", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassSpecialMap::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		return exe.render(ob->adapt(exe, &ClassMap::INSTANCE));
	});
	
	////////////////////////////
	// adaptors
	////////////////////////////
	
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
	
	exe.session.adaptors[&ClassList::INSTANCE][&ClassMap::INSTANCE] = [](Execution& exe, Object* from) {
		Object* result = ClassMap::create();
		auto& map = ClassMap::value(result);
		
		Lock lock{from->mutex};
		for (auto& pair : from->fields(exe)) {
			map[pair.first] = pair.second;
		}
		
		return result;
	};
	
	exe.session.adaptors[&ClassMap::INSTANCE][&ClassList::INSTANCE] = [](Execution& exe, Object* from) {
		Object* result = ClassList::create();
		auto& list = ClassList::value(result);
		
		Lock lock{from->mutex};
		for (Object* val : from->fieldValues(exe)) {
			list.push_back(val);
		}
		
		return result;
	};
	
	exe.session.adaptors[&ClassTime::INSTANCE][&ClassString::INSTANCE] = [](Execution& exe, Object* from) {
		ptime time = ClassTime::value(exe, from);
		return ClassString::create(to_simple_string(time));
	};
	
	exe.session.adaptors[&ClassSpecialMap::INSTANCE][&ClassMap::INSTANCE] = [](Execution& exe, Object* from) {
		auto v = from->fields(exe);
		return ClassMap::create(v.begin(), v.end());
	};
}
