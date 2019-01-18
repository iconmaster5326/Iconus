/*
 * classes.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "classes.hpp"

using namespace std;
using namespace iconus;

iconus::ClassNil iconus::ClassNil::INSTANCE;
iconus::Object iconus::ClassNil::NIL(&INSTANCE);

std::string iconus::ClassNil::name() {
	return "nil";
}

std::string iconus::ClassNil::toString(Object* self) {
	return "nil";
}

iconus::ClassString iconus::ClassString::INSTANCE;

std::string iconus::ClassString::name() {
	return "string";
}

std::string iconus::ClassString::toString(Object* self) {
	string* value = (string*) self->value.asPtr;
	return *value;
}

iconus::ClassSystemFunction iconus::ClassSystemFunction::INSTANCE;

std::string iconus::ClassSystemFunction::name() {
	return "function";
}

bool iconus::ClassSystemFunction::executable() {
	return true;
}

Object* iconus::ClassSystemFunction::execute(Object* self, Scope& scope, Object* input,
		const std::vector<Object*>& args,
		const std::unordered_map<std::string, Object*>& flags) {
	Handler* handler = (Handler*) self->value.asPtr;
	return handler->operator()(scope, input, args, flags);
}
