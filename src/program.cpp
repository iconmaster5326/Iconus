/*
 * program.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include <sstream>

#include "program.hpp"

using namespace std;
using namespace iconus;

iconus::Op::~Op() {
	
}

iconus::OpConst::~OpConst() {
	
}

Object* iconus::OpConst::evaluate(Scope& scope, Object* input) {
	return nullptr;
}

iconus::OpCall::~OpCall() {
	
}

Object* iconus::OpCall::evaluate(Scope& scope, Object* input) {
	return nullptr;
}

iconus::OpBinary::~OpBinary() {
	
}

Object* iconus::OpBinary::evaluate(Scope& scope, Object* input) {
	return nullptr;
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

iconus::Object::operator std::string() {
	return clazz->toString(this);
}

