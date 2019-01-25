/*
 * classes.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "classes.hpp"
#include "error.hpp"
#include "session.hpp"

#include <sstream>


using namespace std;
using namespace iconus;

iconus::ClassNil iconus::ClassNil::INSTANCE{};
iconus::Object iconus::ClassNil::NIL(&ClassNil::INSTANCE);

std::string iconus::ClassNil::name() {
	return "nil";
}

std::string iconus::ClassNil::toString(Object* self, Execution& exe) {
	return "nil";
}

iconus::ClassString iconus::ClassString::INSTANCE{};

std::string iconus::ClassString::name() {
	return "string";
}

std::string iconus::ClassString::toString(Object* self, Execution& exe) {
	string* value = (string*) self->value.asPtr;
	return *value;
}

iconus::ClassSystemFunction iconus::ClassSystemFunction::INSTANCE{};

std::string iconus::ClassSystemFunction::name() {
	return "function";
}

bool iconus::ClassSystemFunction::executable(Object* self, Execution& exe) {
	return true;
}

Object* iconus::ClassSystemFunction::execute(Object* self, Execution& exe, Scope& scope, Object* input,
		Vector<Object*>& args,
		Map<std::string, Object*>& flags) {
	Handler* handler = (Handler*) self->value.asPtr;
	return (*handler)(exe, scope, input, args, flags);
}

iconus::ClassManagedFunction iconus::ClassManagedFunction::INSTANCE{};

Object* iconus::ClassManagedFunction::execute(Object* self, Execution& exe,
		Scope& scope, Object* input, Vector<Object*>& args,
		Map<std::string, Object*>& flags) {
	Instance* instance = (Instance*) self->value.asPtr;
	Map<std::string, Object*> mappedArgs;
	auto argAt = args.begin();
	Map<std::string, Object*> restFlags(flags);
	
	bool inputExplicit = false;
	if (!instance->fn.input.empty()) {
		auto it = flags.find(instance->fn.input);
		if (it != flags.end()) {
			input = it->second;
			inputExplicit = true;
			restFlags.erase(instance->fn.input);
		}
	}
	
	for (const Function::Arg& arg : instance->fn.args) {
		auto it = flags.find(arg.name);
		if (it == flags.end()) {
			if (argAt == args.end()) {
				if (arg.defaultValue) {
					mappedArgs[arg.name] = arg.defaultValue->evaluate(exe, scope, input);
				} else {
					throw Error("Argument '"+arg.name+"' required");
				}
			} else {
				mappedArgs[arg.name] = *argAt;
				argAt++;
			}
		} else {
			mappedArgs[arg.name] = it->second;
			restFlags.erase(arg.name);
		}
	}
	
	for (const Function::Arg& arg : instance->fn.flags) {
		auto it = flags.find(arg.name);
		if (it == flags.end()) {
			if (argAt == args.end()) {
				if (arg.defaultValue) {
					mappedArgs[arg.name] = arg.defaultValue->evaluate(exe, scope, input);
				} else {
					throw Error("Flag '"+arg.name+"' required");
				}
			} else {
				throw Error("Flag '"+arg.name+"' required");
			}
		} else {
			mappedArgs[arg.name] = it->second;
			restFlags.erase(arg.name);
		}
	}
	
	if (instance->fn.vararg.empty()) {
		if (argAt != args.end()) {
			Object* lastArg = *argAt;
			argAt++;
			if (!instance->fn.input.empty() && !inputExplicit && argAt == args.end()) {
				input = lastArg;
			} else {
				throw Error("Too many positional arguments");
			}
		}
	} else {
		auto it = flags.find(instance->fn.vararg);
		if (it == flags.end()) {
			mappedArgs[instance->fn.vararg] = ClassList::create(argAt, args.end());
		} else {
			mappedArgs[instance->fn.vararg] = it->second;
			restFlags.erase(instance->fn.vararg);
			// TODO: unpack list given
			
			if (argAt != args.end()) {
				throw Error("Positional arguments given after vararg argument explicity specified");
			}
		}
	}
	
	if (instance->fn.varflag.empty()) {
		if (!restFlags.empty()) {
			throw Error("Unknown flag '"+restFlags.begin()->first+"'");
		}
	} else {
		auto it = flags.find(instance->fn.varflag);
		if (it == flags.end()) {
			// TODO
		} else {
			mappedArgs[instance->fn.varflag] = it->second;
			restFlags.erase(instance->fn.varflag);
			// TODO: unpack list given
			
			if (!restFlags.empty()) {
				throw Error("More flags given after varflag argument explicity specified");
			}
		}
	}
	
	if (!instance->fn.input.empty()) {
		mappedArgs[instance->fn.input] = input;
	}
	
	Vector<Object*> restArgs(argAt, args.end());
	return instance->handler(exe, scope, input, mappedArgs, restArgs, restFlags);
}

iconus::ClassList iconus::ClassList::INSTANCE{};

std::string iconus::ClassList::name() {
	return "list";
}

std::string iconus::ClassList::toString(Object* self, Execution& exe) {
	ostringstream sb;
	sb << '[';
	
	Deque<Object*>& items = *((Deque<Object*>*)self->value.asPtr);
	bool first = true;
	
	for (Object* ob : items) {
		if (first) {
			first = false;
		} else {
			sb << ", ";
		}
		sb << ob->toString(exe);
	}
	
	sb << ']';
	return sb.str();
}

iconus::ClassError iconus::ClassError::INSTANCE{};

std::string iconus::ClassError::name() {
	return "error";
}

std::string iconus::ClassError::toString(Object* self, Execution& exe) {
	Object* what = (Object*) self->value.asPtr;
	return "error: "+what->toString(exe);
}

iconus::ClassBool iconus::ClassBool::INSTANCE{};
iconus::Object iconus::ClassBool::TRUE(&ClassBool::INSTANCE);
iconus::Object iconus::ClassBool::FALSE(&ClassBool::INSTANCE);

std::string iconus::ClassBool::name() {
	return "bool";
}

std::string iconus::ClassBool::toString(Object* self, Execution& exe) {
	if (self == &TRUE) {
		return "true";
	} else if (self == &FALSE) {
		return "false";
	} else {
		throw runtime_error("bool wansn't TRUE or FALSE");
	}
}

iconus::ClassNumber iconus::ClassNumber::INSTANCE{};

std::string iconus::ClassNumber::name() {
	return "number";
}

std::string iconus::ClassNumber::toString(Object* self, Execution& exe) {
	return to_string(self->value.asDouble);
}

iconus::ClassUserFunction iconus::ClassUserFunction::INSTANCE{};

Object* iconus::ClassUserFunction::create(Scope& scope, Op* op, const Function& fn) {
	Scope* oldScope = &scope;
	return new Object(&ClassUserFunction::INSTANCE, new ClassManagedFunction::Instance(fn, [oldScope,op](auto& exe, auto& evalScope, auto input, auto& args, auto& varargs, auto& varflags) {
		Scope* newScope = new Scope(oldScope, input);
		for (const pair<string,Object*>& kv : args) {
			newScope->setLocal(kv.first, kv.second);
		}
		return op->evaluate(exe, *newScope, input);
	}));
}

Vector<Object*> iconus::ClassList::fieldNames(Object* self, Execution& exe) {
	Deque<Object*>& list = ClassList::value(exe, self);
	Vector<Object*> result;
	
	for (int i = 0; i < list.size(); i++) {
		result.push_back(ClassNumber::create(i));
	}
	
	return result;
}

bool iconus::ClassList::hasField(Object* self, Execution& exe, Object* name) {
	Deque<Object*>& list = ClassList::value(exe, self);
	int i = (int) ClassNumber::value(exe, name);
	
	return i >= 0 && i < list.size();
}

Object* iconus::ClassList::getField(Object* self, Execution& exe, Object* name) {
	Deque<Object*>& list = ClassList::value(exe, self);
	int i = (int) ClassNumber::value(exe, name);
	
	if (i >= 0 && i < list.size()) {
		return list[i];
	} else {
		return Class::getField(self, exe, name);
	}
}

bool iconus::ClassList::canSetField(Object* self, Execution& exe, Object* name) {
	Deque<Object*>& list = ClassList::value(exe, self);
	int i = (int) ClassNumber::value(exe, name);
	
	return i >= 0 && i < list.size();
}

void iconus::ClassList::setField(Object* self, Execution& exe, Object* name, Object* value) {
	Deque<Object*>& list = ClassList::value(exe, self);
	int i = (int) ClassNumber::value(exe, name);
	
	if (i >= 0 && i < list.size()) {
		list[i] = value;
	} else {
		Class::setField(self, exe, name, value);
	}
}

std::size_t iconus::ClassNumber::hash(const Object* self) const {
	return std::hash<double>()(self->value.asDouble);
}

bool iconus::ClassNumber::equals(const Object* self, const Object* other) const {
	return self->value.asDouble == other->value.asDouble;
}

std::size_t iconus::ClassString::hash(const Object* self) const {
	return std::hash<string>()(*(string*)self->value.asPtr);
}

bool iconus::ClassString::equals(const Object* self,
		const Object* other) const {
	const string& a = *(string*)self->value.asPtr;
	const string& b = *(string*)other->value.asPtr;
	return a == b;
}

std::size_t iconus::ClassList::hash(const Object* self) const {
	size_t result = 1;
	Deque<Object*>& value = *(Deque<Object*>*)self->value.asPtr;
	for (const Object* item : value) {
		result *= item->hash();
	}
	return result;
}

bool iconus::ClassList::equals(const Object* self, const Object* other) const {
	Deque<Object*>& a = *(Deque<Object*>*)self->value.asPtr;
	Deque<Object*>& b = *(Deque<Object*>*)other->value.asPtr;
	
	if (a.size() != b.size()) return false;
	for (size_t i = 0; i < a.size(); i++) {
		if (!a[i]->equals(b[i])) return false;
	}
	return true;
}
