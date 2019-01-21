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


using namespace std;
using namespace iconus;

Object* iconus::OpConst::evaluate(Session& session, Scope& scope, Object* input) {
	return value;
}

Object* iconus::OpCall::evaluate(Session& session, Scope& scope, Object* input) {
	Object* cmdOb = scope.get(cmd);
	if (!cmdOb) {
		throw Error("command not in scope: "+cmd);
	}
	if (!cmdOb->executable(session)) {
		throw Error("command not executable: "+cmd);
	}
	
	Vector<Object*> argObs;
	Map<string,Object*> flagObs;
	
	for (const Arg& arg : args) {
		Object* value = arg.value->evaluate(session, scope, input);
		if (arg.isFlag) {
			flagObs[arg.key] = value;
		} else {
			argObs.push_back(value);
		}
	}
	
	return cmdOb->execute(session, scope, input, argObs, flagObs);
}

Object* iconus::OpBinary::evaluate(Session& session, Scope& scope, Object* input) {
	switch (type) {
	case Type::PIPE: {
		Object* lhsResult = lhs ? lhs->evaluate(session, scope, input) : input;
		return rhs ? rhs->evaluate(session, scope, lhsResult) : lhsResult;
	} break;
	case Type::RESET: {
		if (lhs) lhs->evaluate(session, scope, input);
		return rhs ? rhs->evaluate(session, scope, scope.input) : scope.input;
	} break;
	case Type::FOREACH: {
		Object* lhsResult = lhs ? lhs->evaluate(session, scope, input) : input;
		if (rhs) {
			Deque<Object*> foreachResult;
			for (Object* value : lhsResult->fieldValues(session)) {
				foreachResult.push_back(rhs->evaluate(session, scope, value));
			}
			return ClassList::create(foreachResult);
		} else {
			Vector<Object*> v = lhsResult->fieldValues(session);
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

Object* iconus::OpVar::evaluate(Session& session, Scope& scope, Object* input) {
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

Object* iconus::OpLambda::evaluate(Session& session, Scope& scope, Object* input) {
	return ClassUserFunction::create(scope, code, fn);
}

iconus::OpLambda::operator std::string() {
	return "{"+code->operator string()+"}";
}