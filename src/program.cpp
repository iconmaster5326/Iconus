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

bool iconus::Class::executable(Object* self, Execution& exe) {
	return false;
}

Object* iconus::Class::execute(Object* self, Execution& exe, Scope& scope, Object* input,
		Vector<Object*>& args,
		Map<std::string, Object*>& flags) {
	throw Error("cannot execute an object of class "+name());
}

Object* iconus::Scope::get(const std::string& name) {
	Lock lock(mutex);
	
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
	bool inScope; {
		Lock lock(mutex);
		inScope = vars.find(name) == vars.end();
	}
	
	if (inScope) {
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
		Lock lock(mutex);
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
	return exe.adaptionDistance(clazz, to) != -1;
}

Object* iconus::Object::adapt(Execution& exe, Class* to) {
	Adaptor adaptor = exe.getAdaptor(clazz, to);
	if (adaptor) {
		return adaptor(exe, this);
	} else {
		throw Error("Cannot adapt object of class "+clazz->name()+" to class "+to->name());
	}
}

int iconus::Object::adaptionDistance(Execution& exe, Class* clazz) {
	return exe.adaptionDistance(this->clazz, clazz);
}

std::size_t iconus::Class::hash(const Object* self) const {
	return (size_t) self;
}

bool iconus::Class::equals(const Object* self, const Object* other) const {
	return self == other;
}

std::string iconus::Object::toString(Execution& exe) {
	return ClassString::value(exe, this);
}

bool iconus::Object::truthy() {
	return this != &ClassBool::FALSE && this != &ClassNil::NIL;
}

void iconus::Scope::addMethod(const std::string& name, Class* clazz, Object* selector, Object* handler, bool front) {
	auto it = vars.find(name);
	if (it == vars.end()) {
		Object* value = ClassMethod::create();
		ClassMethod::Instance& method = ClassMethod::value(value);
		if (front)
			method.handlers.emplace_front(clazz, selector, handler);
		else
			method.handlers.emplace_back(clazz, selector, handler);
		vars[name] = value;
	} else {
		if (it->second->clazz == &ClassMethod::INSTANCE) {
			ClassMethod::Instance& method = ClassMethod::value(it->second);
			if (front)
				method.handlers.emplace_front(clazz, selector, handler);
			else
				method.handlers.emplace_back(clazz, selector, handler);
		} else {
			throw runtime_error("Attempted to add method "+name+", but was already a value of type "+it->second->clazz->name());
		}
	}
}

void iconus::Scope::addMethod(const std::string& name, Object* handler) {
	auto it = vars.find(name);
	if (it == vars.end()) {
		vars[name] = handler;
	} else {
		if (it->second->clazz == &ClassMethod::INSTANCE) {
			ClassMethod::Instance& method = ClassMethod::value(it->second);
			if (method.defaultHandler) {
				throw runtime_error("Attempted to add default handler to method "+name+", but default handler already defined");
			} else {
				method.defaultHandler = handler;
			}
		} else {
			throw runtime_error("Attempted to add method "+name+", but was already a value of type "+it->second->clazz->name());
		}
	}
}

bool iconus::Class::constructible(Execution& exe) {
	return false;
}

Object* iconus::Class::construct(Execution& exe, Scope& scope,
		Object* input, Vector<Object*>& args,
		Map<std::string, Object*>& flags) {
	throw Error("cannot directly construct an object of class "+name());
}

Vector<Object*> iconus::Class::staticFieldNames(Execution& exe) {
	return Vector<Object*>();
}

Object* iconus::Class::getStaticField(Execution& exe, Object* name) {
	return &ClassNil::NIL;
}

bool iconus::Class::canSetStaticField(Execution& exe, Object* name) {
	return false;
}

void iconus::Class::setStaticField(Execution& exe, Object* name, Object* value) {
	throw Error("Cannot set static field '"+name->toString(exe)+"' on class "+this->name());
}

bool iconus::Class::hasStaticField(Execution& exe, Object* name) {
	Vector<Object*> names = staticFieldNames(exe);
	for (Object* ob : names) {
		if (ob->equals(name)) {
			return true;
		}
	}
	return false;
}

Vector<Object*> iconus::Class::staticFieldValues(Execution& exe) {
	Vector<Object*> result;
	for (Object* name : staticFieldValues(exe)) {
		result.push_back(getStaticField(exe, name));
	}
	return result;
}

Vector<std::pair<Object*, Object*> > iconus::Class::staticFields(Execution& exe) {
	Vector<pair<Object*, Object*> > result;
	for (Object* name : staticFieldNames(exe)) {
		result.push_back(make_pair(name, getStaticField(exe, name)));
	}
	return result;
}
