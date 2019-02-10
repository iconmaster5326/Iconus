/*
 * op.cpp
 *
 *  Created on: Jan 18, 2019
 *      Author: iconmaster
 */

#include "op.hpp"
#include "error.hpp"
#include "classes.hpp"

#include <sstream>
#include <cctype>

using namespace std;
using namespace iconus;

Object* iconus::OpConst::evaluate(Execution& exe, Scope& scope, Object* input) {
	return value;
}

Object* iconus::OpCall::evaluate(Execution& exe, Scope& scope, Object* input) {
	Object* cmdOb = scope.get(cmd);
	if (!cmdOb) {
		throw Error("command not in scope: "+cmd);
	}
	if (!cmdOb->executable(exe)) {
		throw Error("command not executable: "+cmd);
	}
	
	StackTrace::enter(cmd, "input", 1);
	Vector<Object*> argObs;
	Map<string,Object*> flagObs;
	
	for (const Arg& arg : args) {
		Object* value = arg.value->evaluate(exe, scope, input);
		if (arg.isFlag) {
			flagObs[arg.key] = value;
		} else {
			argObs.push_back(value);
		}
	}
	
	Object* result = cmdOb->execute(exe, scope, input, argObs, flagObs);
	StackTrace::exit();
	return result;
}

Object* iconus::OpBinary::evaluate(Execution& exe, Scope& scope, Object* input) {
	switch (type) {
	case Type::PIPE: {
		Object* lhsResult = lhs ? lhs->evaluate(exe, scope, input) : input;
		return rhs ? rhs->evaluate(exe, scope, lhsResult) : lhsResult;
	} break;
	case Type::RESET: {
		if (lhs) lhs->evaluate(exe, scope, input);
		return rhs ? rhs->evaluate(exe, scope, scope.input) : scope.input;
	} break;
	case Type::FOREACH: {
		Object* lhsResult = lhs ? lhs->evaluate(exe, scope, input) : input;
		if (rhs) {
			Deque<Object*> foreachResult;
			for (Object* value : lhsResult->fieldValues(exe)) {
				foreachResult.push_back(rhs->evaluate(exe, scope, value));
			}
			return ClassList::create(foreachResult);
		} else {
			Vector<Object*> v = lhsResult->fieldValues(exe);
			return ClassList::create(v.begin(), v.end());
		}
	} break;
	}
}

iconus::OpConst::operator std::string() {
	return "(const)"; // TODO
}

iconus::OpCall::operator std::string() {
	ostringstream sb;
	sb << '(' << cmd << ' ';
	for (const Arg& arg : args) {
		if (arg.isFlag) {
			sb << '-' << arg.key << ' ' << arg.value->operator string() << ' ';
		} else {
			sb << arg.value->operator string() << ' ';
		}
	}
	sb << ')';
	return sb.str();
}

iconus::OpBinary::operator std::string() {
	ostringstream sb; sb << "[";
	
	if (lhs) {
		sb << lhs->operator string() << " ";
	}
	
	switch (type) {
	case Type::FOREACH: sb << '&'; break;
	case Type::PIPE: sb << '|'; break;
	case Type::RESET: sb << ';'; break;
	}
	
	if (rhs) {
		sb << " " << rhs->operator string();
	}
	
	sb << "]";
	return sb.str();
}

Object* iconus::OpVar::evaluate(Execution& exe, Scope& scope, Object* input) {
	Object* value = scope.get(name);
	if (value) {
		return value;
	} else {
		return &ClassNil::NIL;
	}
}

iconus::OpVar::operator std::string() {
	return "$"+name;
}

Object* iconus::OpLambda::evaluate(Execution& exe, Scope& scope, Object* input) {
	return ClassUserFunction::create(scope, code, fn);
}

iconus::OpLambda::operator std::string() {
	return "{"+code->operator string()+"}";
}

static bool invalidVarChar(char c) {
	static const string badChars{"|&;{}()[]$\'\""};
	return isspace(c) || badChars.find(c) != string::npos;
}

Object* iconus::OpExString::evaluate(Execution& exe, Scope& scope,
		Object* input) {
	Object* resultObject =  ClassString::create("");
	string& result = ClassString::value(resultObject);
	
	bool dollar = false;
	bool dollarExt = false;
	string varname;
	for (char c : str) {
		if (dollar) {
			if (!dollarExt && c == '{' && varname.empty()) {
				dollarExt = true;
			} else if (!dollarExt && invalidVarChar(c)) {
				if (varname.empty()) {
					result += '$';
				} else {
					Object* lookup = scope.get(varname);
					if (lookup) {
						result += lookup->toString(exe);
					}
				}
				
				result += c;
				
				dollar = false;
				varname = "";
			} else if (!dollarExt && c == '$') {
				if (varname.empty()) {
					dollar = false;
					result += '$';
				} else {
					Object* lookup = scope.get(varname);
					if (lookup) {
						result += lookup->toString(exe);
					}
					
					varname = "";
				}
			} else if (dollarExt && c == '{') {
				throw Error("in string constant: cannot have nested '{' in ${...} form");
			} else if (dollarExt && c == '}') {
				Object* lookup = scope.get(varname);
				if (lookup) {
					result += lookup->toString(exe);
				}
				
				dollarExt = false;
				dollar = false;
				varname = "";
			} else {
				varname += c;
			}
		} else if (c == '$') {
			dollar = true;
		} else {
			result += c;
		}
	}
	
	if (dollarExt) throw Error("in string constant: expected '}' at end of string");
	if (dollar && !varname.empty()) {
		Object* lookup = scope.get(varname);
		if (lookup) {
			result += lookup->toString(exe);
		}
	}
	
	return resultObject;
}

iconus::OpExString::operator std::string() {
	return "\""+str+"\"";
}
