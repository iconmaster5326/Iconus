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
#include "session.hpp"

using namespace std;
using namespace iconus;

iconus::Class::~Class() {
	
}

std::string iconus::Class::name() {
	return nullptr;
}

std::string iconus::Class::toString(Object* self, Execution& exe) {
	ostringstream sb;
	sb << name() << '@' << ((void*)self);
	return sb.str();
}

bool iconus::Class::executable(Object* self, Execution& exe) {
	return false;
}

Object* iconus::Class::execute(Object* self, Execution& exe, Scope& scope, Object* input,
		Vector<Object*>& args,
		Map<std::string, Object*>& flags) {
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
			if (parent->canSet(name) && parent->get(name)) {
				parent->set(name, value);
			} else {
				setLocal(name, value);
			}
		} else {
			setLocal(name, value);
		}
	} else {
		setLocal(name, value);
	}
}

void iconus::Scope::setLocal(const std::string& name, Object* value) {
	if (canSet(name)) {
		vars[name] = value;
	} else {
		throw Error("Field '"+name+"' of this scope cannot be set");
	}
}

bool iconus::Scope::canSet(const std::string& name) {
	return true;
}

Vector<Object*> iconus::Class::fieldNames(Object* self, Execution& exe) {
	return Vector<Object*>();
}

bool iconus::Class::hasField(Object* self, Execution& exe, Object* name) {
	Vector<Object*> names = fieldNames(self, exe);
	for (Object* ob : names) {
		if (ob->equals(name)) {
			return true;
		}
	}
	return false;
}

Object* iconus::Class::getField(Object* self, Execution& exe, Object* name) {
	return &ClassNil::NIL;
}

bool iconus::Class::canSetField(Object* self, Execution& exe, Object* name) {
	return false;
}

void iconus::Class::setField(Object* self, Execution& exe, Object* name, Object* value) {
	throw Error("Cannot set field '"+name->toString(exe)+"' on object of class "+self->clazz->name());
}

iconus::Scope::Scope() : parent(nullptr), input(&ClassNil::NIL) {}
iconus::Scope::Scope(Scope* parent) : parent(parent), input(&ClassNil::NIL) {}
iconus::Scope::Scope(Scope* parent, Object* input) : parent(parent), input(input) {}
iconus::Scope::~Scope() {}

Vector<Object*> iconus::Class::fieldValues(Object* self, Execution& exe) {
	Vector<Object*> result;
	for (Object* name : fieldNames(self, exe)) {
		result.push_back(getField(self, exe, name));
	}
	return result;
}

Vector<std::pair<Object*, Object*> > iconus::Class::fields(Object* self, Execution& exe) {
	Vector<pair<Object*, Object*> > result;
	for (Object* name : fieldNames(self, exe)) {
		result.push_back(make_pair(name, getField(self, exe, name)));
	}
	return result;
}

bool iconus::Object::adaptableTo(Execution& exe, Class* to) {
	return exe.getAdaptor(clazz, to) != nullptr;
}

Object* iconus::Object::adapt(Execution& exe, Class* to) {
	Adaptor adaptor = exe.getAdaptor(clazz, to);
	if (adaptor) {
		return adaptor(exe, this);
	} else {
		throw Error("Cannot adapt object of class "+clazz->name()+" to class "+to->name());
	}
}

std::size_t iconus::Class::hash(const Object* self) const {
	return (size_t) self;
}

bool iconus::Class::equals(const Object* self, const Object* other) const {
	return self == other;
}
