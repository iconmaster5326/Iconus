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

std::string iconus::Class::toString(Object* self, Session& session) {
	ostringstream sb;
	sb << name() << '@' << ((void*)self);
	return sb.str();
}

bool iconus::Class::executable(Object* self, Session& session) {
	return false;
}

Object* iconus::Class::execute(Object* self, Session& session, Scope& scope, Object* input,
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

Vector<Object*> iconus::Class::fieldNames(Object* self, Session& session) {
	return Vector<Object*>();
}

bool iconus::Class::hasField(Object* self, Session& session, Object* name) {
	return false;
}

Object* iconus::Class::getField(Object* self, Session& session, Object* name) {
	return &ClassNil::NIL;
}

bool iconus::Class::canSetField(Object* self, Session& session, Object* name) {
	return false;
}

void iconus::Class::setField(Object* self, Session& session, Object* name, Object* value) {
	throw Error("Cannot set field '"+name->toString(session)+"' on object of class "+self->clazz->name());
}

iconus::Scope::Scope() : parent(nullptr), input(&ClassNil::NIL) {}
iconus::Scope::Scope(Scope* parent) : parent(parent), input(&ClassNil::NIL) {}
iconus::Scope::Scope(Scope* parent, Object* input) : parent(parent), input(input) {}

Vector<Object*> iconus::Class::fieldValues(Object* self, Session& session) {
	Vector<Object*> result;
	for (Object* name : fieldNames(self, session)) {
		result.push_back(getField(self, session, name));
	}
	return result;
}

Vector<std::pair<Object*, Object*> > iconus::Class::fields(Object* self, Session& session) {
	Vector<pair<Object*, Object*> > result;
	for (Object* name : fieldNames(self, session)) {
		result.push_back(make_pair(name, getField(self, session, name)));
	}
	return result;
}

bool iconus::Object::adaptableTo(Session& session, Class* to) {
	return session.getAdaptor(clazz, to) != nullptr;
}

Object* iconus::Object::adapt(Session& session, Class* to) {
	Adaptor adaptor = session.getAdaptor(clazz, to);
	if (adaptor) {
		return adaptor(session, this);
	} else {
		throw Error("Cannot adapt object of class "+clazz->name()+" to class "+to->name());
	}
}
