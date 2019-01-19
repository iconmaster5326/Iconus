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

iconus::Op::~Op() {
	
}

iconus::OpConst::~OpConst() {
	
}

Object* iconus::OpConst::evaluate(Session& session, Scope& scope, Object* input) {
	return value;
}

iconus::OpCall::~OpCall() {
	
}

Object* iconus::OpCall::evaluate(Session& session, Scope& scope, Object* input) {
	Object* cmdOb = scope.get(cmd);
	if (!cmdOb) {
		throw Error("command not in scope: "+cmd);
	}
	if (!cmdOb->executable()) {
		throw Error("command not executable: "+cmd);
	}
	
	vector<Object*> argObs;
	unordered_map<string,Object*> flagObs;
	
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

iconus::OpBinary::~OpBinary() {
	
}

Object* iconus::OpBinary::evaluate(Session& session, Scope& scope, Object* input) {
	switch (type) {
	case Type::PIPE: {
		Object* lhsResult = lhs ? lhs->evaluate(session, scope, input) : input;
		return rhs ? rhs->evaluate(session, scope, lhsResult) : lhsResult;
	} break;
	default: throw exception();
	}
}

iconus::OpConst::operator std::string() {
	return value->operator string();
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

iconus::OpVar::~OpVar() {
	
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

iconus::OpLambda::~OpLambda() {
	
}

Object* iconus::OpLambda::evaluate(Session& session, Scope& scope, Object* input) {
	return ClassUserFunction::create(scope, code, fn);
}

iconus::OpLambda::operator std::string() {
	return "{"+code->operator string()+"}";
}
