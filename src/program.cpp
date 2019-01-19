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

iconus::Class::~Class() {
	
}

std::string iconus::Class::name() {
	return nullptr;
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

Object* iconus::Scope::get(const std::string& name) {
	auto it = vars.find(name);
	if (it == vars.end()) {
		if (parent) {
			return parent->get(name);
		} else {
			return nullptr;
		}
	} else {
		return it->second;
	}
}

void iconus::Scope::set(const std::string& name, Object* value) {
	if (vars.find(name) == vars.end()) {
		if (parent) {
			if (parent->get(name)) {
				parent->set(name, value);
			} else {
				vars[name] = value;
			}
		} else {
			vars[name] = value;
		}
	} else {
		vars[name] = value;
	}
}

void iconus::Scope::setLocal(const std::string& name, Object* value) {
	vars[name] = value;
}

std::vector<Object*> iconus::Class::fieldNames(Object* self) {
	return vector<Object*>();
}

bool iconus::Class::hasField(Object* self, Object* name) {
	return false;
}

Object* iconus::Class::getField(Object* self, Object* name) {
	return &ClassNil::NIL;
}

bool iconus::Class::canSetField(Object* self, Object* name) {
	return false;
}

void iconus::Class::setField(Object* self, Object* name, Object* value) {
	throw Error("Cannot set field '"+name->operator string()+"' on object of class "+self->clazz->name());
}

Object* iconus::Object::castTo(Object* ob, Class* clazz) {
	if (ob->clazz == clazz) {
		return ob;
	} else {
		throw Error("Cannot cast object of class "+ob->clazz->name()+" to class "+clazz->name());
	}
}
