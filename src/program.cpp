/*
 * program.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include <sstream>

#include "program.hpp"
#include "classes.hpp"

using namespace std;
using namespace iconus;

iconus::Op::~Op() {
	
}

iconus::OpConst::~OpConst() {
	
}

Object* iconus::OpConst::evaluate(Scope& scope, Object* input) {
	return value;
}

iconus::OpCall::~OpCall() {
	
}

Object* iconus::OpCall::evaluate(Scope& scope, Object* input) {
	ostringstream sb; sb << cmd << "(" << (input ? input->operator string() : "nil") << "|";
	for (const Arg& arg : args) {
		Object* v = arg.value->evaluate(scope, input);
		sb << (v ? v->operator string() : "nil");
	}
	sb << ")";
	return new Object(&ClassString::INSTANCE, new string(sb.str()));
}

iconus::OpBinary::~OpBinary() {
	
}

Object* iconus::OpBinary::evaluate(Scope& scope, Object* input) {
	switch (type) {
	case Type::PIPE:
		return rhs->evaluate(scope, lhs->evaluate(scope, input));
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

Object* iconus::Class::execute(Object* self, Scope& scope, Object* input,
		const std::vector<Object*>& args,
		const std::unordered_map<std::string, Object*>& flags) {
	throw runtime_error("cannot execute an object of class "+name());
}
