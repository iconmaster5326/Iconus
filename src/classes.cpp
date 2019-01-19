/*
 * classes.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "classes.hpp"
#include "error.hpp"

#include <sstream>
#include <deque>

using namespace std;
using namespace iconus;

iconus::ClassNil iconus::ClassNil::INSTANCE{};
iconus::Object iconus::ClassNil::NIL(&ClassNil::INSTANCE);

std::string iconus::ClassNil::name() {
	return "nil";
}

std::string iconus::ClassNil::toString(Object* self) {
	return "nil";
}

iconus::ClassString iconus::ClassString::INSTANCE{};

std::string iconus::ClassString::name() {
	return "string";
}

std::string iconus::ClassString::toString(Object* self) {
	string* value = (string*) self->value.asPtr;
	return *value;
}

iconus::ClassSystemFunction iconus::ClassSystemFunction::INSTANCE{};

std::string iconus::ClassSystemFunction::name() {
	return "function";
}

bool iconus::ClassSystemFunction::executable() {
	return true;
}

Object* iconus::ClassSystemFunction::execute(Object* self, Session& session, Scope& scope, Object* input,
		const std::vector<Object*>& args,
		const std::unordered_map<std::string, Object*>& flags) {
	Handler* handler = (Handler*) self->value.asPtr;
	return handler->operator()(session, scope, input, args, flags);
}

iconus::ClassManagedFunction iconus::ClassManagedFunction::INSTANCE{};

Object* iconus::ClassManagedFunction::execute(Object* self, Session& session,
		Scope& scope, Object* input, const std::vector<Object*>& args,
		const std::unordered_map<std::string, Object*>& flags) {
	Instance* instance = (Instance*) self->value.asPtr;
	std::unordered_map<std::string, Object*> mappedArgs;
	auto argAt = args.begin();
	std::unordered_map<std::string, Object*> restFlags(flags);
	
	bool inputExplicit = false;
	if (!instance->input.empty()) {
		auto it = flags.find(instance->input);
		if (it != flags.end()) {
			input = it->second;
			inputExplicit = true;
			restFlags.erase(instance->input);
		}
	}
	
	for (const Instance::Arg& arg : instance->args) {
		auto it = flags.find(arg.name);
		if (it == flags.end()) {
			if (argAt == args.end()) {
				if (arg.defaultValue) {
					mappedArgs[arg.name] = arg.defaultValue;
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
	
	if (instance->vararg.empty()) {
		if (argAt != args.end()) {
			Object* lastArg = *argAt;
			argAt++;
			if (!inputExplicit && argAt == args.end()) {
				input = lastArg;
			} else {
				throw Error("Too many positional arguments");
			}
		}
	} else {
		auto it = flags.find(instance->vararg);
		if (it == flags.end()) {
			mappedArgs[instance->vararg] = new Object(&ClassList::INSTANCE, new deque<Object*>(argAt, args.end()));
		} else {
			mappedArgs[instance->vararg] = it->second;
			restFlags.erase(instance->vararg);
			// TODO: unpack list given
			
			if (argAt != args.end()) {
				throw Error("Positional arguments given after vararg argument explicity specified");
			}
		}
	}
	
	if (instance->varflag.empty()) {
		if (!restFlags.empty()) {
			throw Error("Unknown flag '"+restFlags.begin()->first+"'");
		}
	} else {
		auto it = flags.find(instance->varflag);
		if (it == flags.end()) {
			// TODO
		} else {
			mappedArgs[instance->varflag] = it->second;
			restFlags.erase(instance->varflag);
			// TODO: unpack list given
			
			if (!restFlags.empty()) {
				throw Error("More flags given after varflag argument explicity specified");
			}
		}
	}
	
	if (!instance->input.empty()) {
		mappedArgs[instance->input] = input;
	}
	
	vector<Object*> restArgs(argAt, args.end());
	return instance->handler(session, scope, input, mappedArgs, restArgs, restFlags);
}

iconus::ClassList iconus::ClassList::INSTANCE{};

std::string iconus::ClassList::name() {
	return "list";
}

std::string iconus::ClassList::toString(Object* self) {
	ostringstream sb;
	sb << '[';
	
	deque<Object*>& items = *((deque<Object*>*)self->value.asPtr);
	bool first = true;
	
	for (Object* ob : items) {
		if (first) {
			first = false;
		} else {
			sb << ", ";
		}
		sb << ob->operator string();
	}
	
	sb << ']';
	return sb.str();
}

iconus::ClassError iconus::ClassError::INSTANCE{};

std::string iconus::ClassError::name() {
	return "error";
}

std::string iconus::ClassError::toString(Object* self) {
	Object* what = (Object*) self->value.asPtr;
	return "error: "+what->operator string();
}

iconus::ClassBool iconus::ClassBool::INSTANCE{};
iconus::Object iconus::ClassBool::TRUE(&ClassBool::INSTANCE);
iconus::Object iconus::ClassBool::FALSE(&ClassBool::INSTANCE);

std::string iconus::ClassBool::name() {
	return "bool";
}

std::string iconus::ClassBool::toString(Object* self) {
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

std::string iconus::ClassNumber::toString(Object* self) {
	return to_string(self->value.asDouble);
}
