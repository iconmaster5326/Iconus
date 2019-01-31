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
#include <iostream>

using namespace std;
using namespace iconus;

iconus::ClassNil iconus::ClassNil::INSTANCE{};
iconus::Object iconus::ClassNil::NIL(&ClassNil::INSTANCE);

std::string iconus::ClassNil::name() {
	return "nil";
}

std::string iconus::ClassNil::toString(Object* self, Execution& exe) {
	return "";
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
	using Arg = Function::Arg;
	using Role = Function::Role;
	
	// INVARIANTS:
	// There will only be 0 or 1 input, vararg, and varflag args, each.
	// There will never be both an input arg and a vararg. (there may, however, be an input flag and a vararg).
	// Inputs, varargs, and varflags will never have default values.
	// Varargs may only appear in the args. Varflags may only appear in the flags. Input may appear in either.
	
	// RESULTS:
	// Args fill up from left to right.
	// Varargs is non-greedy; that is, will leave values for required parameters to thier right.
	// Optional values around varargs get thier values filled (from left to right) before varargs does.
	// The input arg will always be matched last.
	// Flags and args always get thier values assigned before varflags.
	// If the input was matched, it will change the input as well as assign the variable, and vice versa if not matched.
	// Flags go to args before args do.
	// Required args fill up left to right before optional ones, but arg order is maintained.
	
	Instance* instance = (Instance*) self->value.asPtr;
	Map<std::string, Object*> mappedArgs;
	Map<std::string, Object*> varflags;
	
	bool inputIsArg;
	Arg* inputArg = nullptr;
	Arg* varargArg = nullptr;
	Arg* varflagArg = nullptr;
	
	for (Arg& arg : instance->fn.args) {
		switch (arg.role) {
		case Role::INPUT: inputArg = &arg; inputIsArg = true; break;
		case Role::VARARG: varargArg = &arg; break;
		}
	}
	for (Arg& arg : instance->fn.flags) {
		switch (arg.role) {
		case Role::INPUT: inputArg = &arg; inputIsArg = false; break;
		case Role::VARFLAG: varflagArg = &arg; break;
		}
	}
	
	for (Arg& arg : instance->fn.args) {
		auto it = flags.find(arg.name);
		if (it != flags.end()) {
			mappedArgs[arg.name] = it->second;
			if (arg.role == Role::INPUT) input = it->second;
			// TODO: what if varargs and varflags
			flags.erase(arg.name);
		}
	}
	for (Arg& arg : instance->fn.flags) {
		auto it = flags.find(arg.name);
		if (it != flags.end()) {
			mappedArgs[arg.name] = it->second;
			flags.erase(arg.name);
		} else if (arg.defaultValue) {
			mappedArgs[arg.name] = arg.defaultValue->evaluate(exe, scope, input);
			flags.erase(arg.name);
		} else if (arg.role == Role::NONE) {
			throw Error("Flag '"+arg.name+"' required");
		}
	}
	
	int requiredArgs = 0;
	int optionalArgs = 0;
	for (Arg& arg : instance->fn.args) {
		if (arg.role != Role::NONE || mappedArgs.find(arg.name) != mappedArgs.end()) continue;
		if (arg.defaultValue) {
			optionalArgs++;
		} else {
			requiredArgs++;
		}
	}
	
	int overflow1 = args.size() - requiredArgs;
	int overflow2 = overflow1 - optionalArgs;
	
	if (overflow1 < 0) throw Error("Not enough arguments given");
	if ((inputArg && overflow2 > 1) || (!inputArg && !varargArg && overflow2 > 0)) throw Error("Too many arguments given");
	
	Vector<Arg*> postArgs; bool post = false;
	int optionalsToHave = overflow1;
	
	int argAt = 0;
	int preI, postI;
	for (Arg& arg : instance->fn.args) {
		if (arg.role != Role::NONE) {
			preI = argAt;
			post = true;
			continue;
		}
		
		if (mappedArgs.find(arg.name) != mappedArgs.end()) continue;
		
		if (arg.defaultValue && optionalsToHave <= 0) {
			mappedArgs[arg.name] = arg.defaultValue->evaluate(exe, scope, input);
			continue;
		}
		
		if (post) {
			postArgs.push_back(&arg);
		} else {
			mappedArgs[arg.name] = args[argAt];
			argAt++;
			if (arg.defaultValue) optionalsToHave--;
		}
	}
	
	postI = argAt = args.size() - postArgs.size();
	for (Arg* arg : postArgs) {
		if (mappedArgs.find(arg->name) != mappedArgs.end()) continue;
		
		if (arg->defaultValue && optionalsToHave <= 0) {
			mappedArgs[arg->name] = arg->defaultValue->evaluate(exe, scope, input);
			continue;
		}
		
		mappedArgs[arg->name] = args[argAt];
		argAt++;
		
		if (arg->defaultValue) optionalsToHave--;
	}
	
	Vector<Object*> varargs;
	if (inputArg) {
		if (optionalsToHave == 1) {
			input = mappedArgs[inputArg->name] = args[preI];
		} else {
			mappedArgs[inputArg->name] = input;
		}
	} else if (varargArg) {
		varargs.insert(varargs.begin(), args.begin()+preI, args.begin()+postI);
		// TODO: assign to mappedArgs
	}
	
	if (varflagArg) {
		// TODO: assign to mappedArgs
	} else if (!flags.empty()) {
		throw Error("Too many flags given");
	}
	
	return instance->handler(exe, scope, input, mappedArgs, varargs, flags);
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
	string s = to_string(self->value.asDouble);
	if (s.find('.') != string::npos) {
		while (s.back() == '0' || s.back() == '.') {
			char c = s.back();
			s.pop_back();
			if (c == '.') break;
		}
	}
	return s.empty() ? "0" : s;
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

iconus::ClassClass iconus::ClassClass::INSTANCE{};

std::string iconus::ClassClass::name() {
	return "class";
}

std::string iconus::ClassClass::toString(Object* self, Execution& exe) {
	Class* value = (Class*) self->value.asPtr;
	return "class "+value->name();
}

std::size_t iconus::ClassClass::hash(const Object* self) const {
	return (size_t) self->value.asPtr;
}

bool iconus::ClassClass::equals(const Object* self, const Object* other) const {
	return self->value.asPtr == other->value.asPtr;
}

iconus::ClassMap iconus::ClassMap::INSTANCE{};

std::string iconus::ClassMap::name() {
	return "map";
}

std::string iconus::ClassMap::toString(Object* self, Execution& exe) {
	return "(map...)";
}

std::size_t iconus::ClassMap::hash(const Object* self) const {
	size_t hash = 1;
	for (auto& pair : ClassMap::value(self)) {
		hash *= pair.first->hash() * pair.second->hash();
	}
	return hash;
}

bool iconus::ClassMap::equals(const Object* self, const Object* other) const {
	auto& a = ClassMap::value(self);
	auto& b = ClassMap::value(other);
	if (a.size() != b.size()) return false;
	for (auto& pair : a) {
		if (b.find(pair.first) == b.end()) return false;
		if (a[pair.first] != b[pair.first]) return false;
	}
	for (auto& pair : b) {
		if (a.find(pair.first) == a.end()) return false;
		if (a[pair.first] != b[pair.first]) return false;
	}
	return true;
}

Vector<Object*> iconus::ClassMap::fieldNames(Object* self, Execution& exe) {
	Vector<Object*> result;
	for (auto& pair : ClassMap::value(exe, self)) {
		result.push_back(pair.first);
	}
	return result;
}

Object* iconus::ClassMap::getField(Object* self, Execution& exe, Object* name) {
	auto& map = ClassMap::value(exe, self);
	auto it = map.find(name);
	if (it == map.end()) {
		return &ClassNil::NIL;
	} else {
		return it->second;
	}
}

bool iconus::ClassMap::canSetField(Object* self, Execution& exe, Object* name) {
	return true;
}

void iconus::ClassMap::setField(Object* self, Execution& exe, Object* name,
		Object* value) {
	auto& map = ClassMap::value(exe, self);
	map[name] = value;
}
