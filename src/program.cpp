/*
 * program.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include <sstream>

#include "program.hpp"
#include "classes.hpp"
#include "error.hpp"

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
	auto it = scope.vars.find(cmd);
	if (it == scope.vars.end()) {
		throw Error("command not in scope: "+cmd);
	}
	Object* cmdOb = it->second;
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

iconus::Class::~Class() {
	
}

std::string iconus::Class::name() {
	return nullptr;
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

std::string iconus::Class::toString(Object* self) {
	ostringstream sb;
	sb << name() << '@' << ((void*)self);
	return sb.str();
}

bool iconus::Class::executable() {
	return false;
}

Object* iconus::Class::execute(Object* self, Session& session, Scope& scope, Object* input,
		const std::vector<Object*>& args,
		const std::unordered_map<std::string, Object*>& flags) {
	throw Error("cannot execute an object of class "+name());
}
