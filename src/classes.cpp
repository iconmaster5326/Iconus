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
#include <limits>

using namespace std;
using namespace iconus;

iconus::ClassNil iconus::ClassNil::INSTANCE{};
iconus::Object iconus::ClassNil::NIL(&ClassNil::INSTANCE);

#define DEF_FIELD(str,name) static string name##_S{str}; static Object name{&ClassString::INSTANCE, &name##_S}

std::string iconus::ClassNil::name() {
	return "nil";
}

iconus::ClassString iconus::ClassString::INSTANCE{};

std::string iconus::ClassString::name() {
	return "str";
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
	// There will never be both an input arg and a vararg. (there may, however, be an input flag and a varflag).
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
	
	Lock lock{self->mutex};
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
	
	Vector<Object*> varargs;
	Map<Object*,Object*> givenVarflags;
	bool explicitVarargs = false;
	bool explicitVarflags = false;
	for (Arg& arg : instance->fn.args) {
		auto it = flags.find(arg.name);
		if (it != flags.end()) {
			mappedArgs[arg.name] = it->second;
			if (arg.role == Role::INPUT) {
				input = it->second;
			} else if (arg.role == Role::VARARG) {
				Lock listLock{it->second->mutex};
				
				explicitVarargs = true;
				auto& value = ClassList::value(exe, it->second);
				varargs.insert(varargs.end(), value.begin(), value.end());
			}
			flags.erase(arg.name);
		}
	}
	for (Arg& arg : instance->fn.flags) {
		auto it = flags.find(arg.name);
		if (it != flags.end()) {
			mappedArgs[arg.name] = it->second;
			
			if (arg.role == Role::VARFLAG) {
				Lock listLock{it->second->mutex};
				
				explicitVarflags = true;
				auto& value = ClassMap::value(exe, it->second);
				givenVarflags.insert(value.begin(), value.end());
			}
			
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
	if ((inputArg && overflow2 > 1) || (!inputArg && !varargArg && overflow2 > 0) || (explicitVarargs && overflow2 > 0)) throw Error("Too many arguments given");
	
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
	
	if (inputArg) {
		if (optionalsToHave == 1) {
			input = mappedArgs[inputArg->name] = args[preI];
		} else {
			mappedArgs[inputArg->name] = input;
		}
	} else if (varargArg) {
		varargs.insert(varargs.begin(), args.begin()+preI, args.begin()+postI);
		if (!explicitVarargs) {
			mappedArgs[varargArg->name] = ClassList::create(varargs.begin(), varargs.end());
		}
	}
	
	if (varflagArg && !explicitVarflags) {
		Object* mapObj = ClassMap::create();
		auto& map = ClassMap::value(mapObj);
		
		for (auto& pair : flags) {
			map[ClassString::create(pair.first)] = pair.second;
		}
		
		mappedArgs[varflagArg->name] = mapObj;
	} else if (!flags.empty()) {
		throw Error("Too many flags given");
	}
	
	if (explicitVarflags) {
		for (auto& pair : givenVarflags) {
			flags[pair.first->toString(exe)] = pair.second;
		}
	}
	
	return instance->handler(exe, scope, input, mappedArgs, varargs, flags);
}

iconus::ClassList iconus::ClassList::INSTANCE{};

std::string iconus::ClassList::name() {
	return "list";
}

iconus::ClassError iconus::ClassError::INSTANCE{};

std::string iconus::ClassError::name() {
	return "error";
}

iconus::ClassBool iconus::ClassBool::INSTANCE{};
iconus::Object iconus::ClassBool::TRUE(&ClassBool::INSTANCE);
iconus::Object iconus::ClassBool::FALSE(&ClassBool::INSTANCE);

std::string iconus::ClassBool::name() {
	return "bool";
}

iconus::ClassNumber iconus::ClassNumber::INSTANCE{};

std::string iconus::ClassNumber::name() {
	return "num";
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
	if (!self) return Class::fieldNames(self, exe);
	
	Lock lock{self->mutex};
	Deque<Object*>& list = ClassList::value(exe, self);
	Vector<Object*> result;
	
	for (int i = 0; i < list.size(); i++) {
		result.push_back(ClassNumber::create(i));
	}
	
	return result;
}

bool iconus::ClassList::hasField(Object* self, Execution& exe, Object* name) {
	Lock lock{self->mutex};
	Deque<Object*>& list = ClassList::value(exe, self);
	int i = (int) ClassNumber::value(exe, name);
	
	return i >= 0 && i < list.size();
}

Object* iconus::ClassList::getField(Object* self, Execution& exe, Object* name) {
	Lock lock{self->mutex};
	Deque<Object*>& list = ClassList::value(exe, self);
	int i = (int) ClassNumber::value(exe, name);
	
	if (i >= 0 && i < list.size()) {
		return list[i];
	} else {
		return Class::getField(self, exe, name);
	}
}

bool iconus::ClassList::canSetField(Object* self, Execution& exe, Object* name) {
	Lock lock{self->mutex};
	Deque<Object*>& list = ClassList::value(exe, self);
	int i = (int) ClassNumber::value(exe, name);
	
	return i >= 0 && i < list.size();
}

void iconus::ClassList::setField(Object* self, Execution& exe, Object* name, Object* value) {
	Lock lock{self->mutex};
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
	if (!self) return Class::fieldNames(self, exe);
	
	Lock lock{self->mutex};
	Vector<Object*> result;
	for (auto& pair : ClassMap::value(exe, self)) {
		result.push_back(pair.first);
	}
	return result;
}

Object* iconus::ClassMap::getField(Object* self, Execution& exe, Object* name) {
	Lock lock{self->mutex};
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
	Lock lock{self->mutex};
	auto& map = ClassMap::value(exe, self);
	map[name] = value;
}

iconus::ClassTable iconus::ClassTable::INSTANCE{};

std::string iconus::ClassTable::name() {
	return "table";
}

iconus::ClassMethod iconus::ClassMethod::INSTANCE{};

Object* iconus::ClassMethod::execute(Object* self, Execution& exe, Scope& scope,
		Object* input, Vector<Object*>& args,
		Map<std::string, Object*>& flags) {
	Lock lock{self->mutex};
	Instance& method = ClassMethod::value(exe, self);
	
	int bestDist = numeric_limits<int>::max();
	Object* bestHandler = nullptr;
	for (auto& handler : method.handlers) {
		Object* selectedOb;
		try {
			selectedOb = handler.selector->execute(exe, scope, input, args, flags);
		} catch (const Error& e) {
			continue;
		}
		
		Class* selected;
		try {
			selected = ClassClass::value(exe, selectedOb);
		} catch (const Error& e) {
			throw Error("Method selector returned an object of class "+selectedOb->clazz->name()+" (expected <class>)");
		}
		
		int dist = exe.adaptionDistance(selected, handler.clazz);
		if (dist != -1 && dist < bestDist) {
			bestDist = dist;
			bestHandler = handler.handler;
		}
	}
	
	if (bestHandler) {
		return bestHandler->execute(exe, scope, input, args, flags);
	} else if (method.defaultHandler) {
		return method.defaultHandler->execute(exe, scope, input, args, flags);
	} else {
		throw Error("Method call not applicable");
	}
}

iconus::ClassTime iconus::ClassTime::INSTANCE{};

std::string iconus::ClassTime::name() {
	return "time";
}

std::string iconus::ClassUserDefined::name() {
	return className;
}

Vector<Object*> iconus::ClassUserDefined::fieldNames(Object* self,
		Execution& exe) {
	return classFields;
}

Object* iconus::ClassUserDefined::getField(Object* self, Execution& exe,
		Object* name) {
	Lock lock{self->mutex};
	Instance& map = value(self);
	auto it = map.find(name);
	return it != map.end() ? it->second : Class::getField(self, exe, name);
}

bool iconus::ClassUserDefined::canSetField(Object* self, Execution& exe,
		Object* name) {
	auto it = find_if(classFields.begin(), classFields.end(), [&](Object* val) {
		return name->equals(val);
	});
	return it != classFields.end();
}

void iconus::ClassUserDefined::setField(Object* self, Execution& exe,
		Object* name, Object* value) {
	if (canSetField(self,exe,name)) {
		Lock lock{self->mutex};
		Instance& map = this->value(self);
		map[name] = value;
	} else {
		return Class::setField(self, exe, name, value);
	}
}

bool iconus::ClassClass::executable(Object* self, Execution& exe) {
	return true;
}

Object* iconus::ClassClass::execute(Object* self, Execution& exe, Scope& scope,
		Object* input, Vector<Object*>& args,
		Map<std::string, Object*>& flags) {
	Class* clazz = ClassClass::value(self);
	return clazz->construct(exe, scope, input, args, flags);
}

bool iconus::ClassUserDefined::constructible(Execution& exe) {
	return true;
}

Object* iconus::ClassUserDefined::construct(Execution& exe, Scope& scope,
		Object* input, Vector<Object*>& args,
		Map<std::string, Object*>& flags) {
	Object* result = create();
	auto& map = value(result);
	
	for (auto& pair : flags) {
		Object* key = ClassString::create(pair.first);
		auto it = find_if(classFields.begin(), classFields.end(), [&](Object* val) {
			return key->equals(val);
		});
		if (it == classFields.end()) {
			throw Error("Flag does not correspond to a field in class "+className+": -"+pair.first);
		} else {
			map[key] = pair.second;
		}
	}
	
	int i = 0; while (i < classFields.size() && map.find(classFields[i]) != map.end()) i++;
	for (Object* arg : args) {
		if (i >= classFields.size()) throw Error("Too many arguments given to constructor of class "+className);
		map[classFields[i]] = arg;
		while (i < classFields.size() && map.find(classFields[i]) != map.end()) i++;
	}
	
	for (Object* field : classFields) {
		if (map.find(field) == map.end()) {
			map[field] = &ClassNil::NIL;
		}
	}
	
	return result;
}

Vector<Object*> iconus::ClassUserDefined::staticFieldNames(Execution& exe) {
	Vector<Object*> result;
	for (auto& pair : classStaticFields) {
		result.push_back(pair.first);
	}
	return result;
}

Object* iconus::ClassUserDefined::getStaticField(Execution& exe, Object* name) {
	auto it = classStaticFields.find(name);
	if (it == classStaticFields.end()) {
		return Class::getStaticField(exe, name);
	} else {
		return it->second;
	}
}

bool iconus::ClassUserDefined::canSetStaticField(Execution& exe, Object* name) {
	return true;
}

void iconus::ClassUserDefined::setStaticField(Execution& exe, Object* name,
		Object* value) {
	classStaticFields[name] = value;
}

DEF_FIELD("name",CLASS_FIELD_NAME);
DEF_FIELD("fields",CLASS_FIELD_FIELDS);
DEF_FIELD("adaptors",CLASS_FIELD_ADAPTS);

Vector<Object*> iconus::ClassClass::fieldNames(Object* self, Execution& exe) {
	if (self) {
		Class* clazz = ClassClass::value(self);
		Vector<Object*> v{{&CLASS_FIELD_NAME, &CLASS_FIELD_FIELDS, &CLASS_FIELD_ADAPTS}};
		auto v2 = clazz->staticFieldNames(exe);
		v.insert(v.begin(), v2.begin(), v2.end());
		return v;
	} else {
		return Vector<Object*>({&CLASS_FIELD_NAME, &CLASS_FIELD_FIELDS, &CLASS_FIELD_ADAPTS});
	}
}

Object* iconus::ClassClass::getField(Object* self, Execution& exe,
		Object* name) {
	Class* clazz = ClassClass::value(self);
	
	if (name->equals(&CLASS_FIELD_NAME)) {
		return ClassString::create(clazz->name());
	} else if (name->equals(&CLASS_FIELD_FIELDS)) {
		Vector<Object*> fs = clazz->fieldNames(nullptr, exe);
		return ClassList::create(fs.begin(), fs.end());
	} else if (name->equals(&CLASS_FIELD_ADAPTS)) {
		return ClassSpecialMap::create([clazz](Execution& exe, Scope& scope, Object* input, auto& out) {
			for (auto& adaptor : exe.session.adaptors[clazz]) {
				out.emplace_back(ClassClass::create(adaptor.first));
			}
		}, [clazz](Execution& exe, Scope& scope, Object* input, Object* name) {
			if (!name->adaptableTo(exe, &ClassClass::INSTANCE)) return &ClassNil::NIL;
			Class* otherClass = ClassClass::value(exe, name);
			
			auto& classes = exe.session.adaptors[clazz];
			auto it = classes.find(otherClass);
			
			if (it == classes.end()) {
				return &ClassNil::NIL;
			} else {
				return ClassSystemFunction::create([it](Execution& exe, Scope& scope, Object* input, auto& args, auto& flags) {
					return it->second(exe, input);
				});
			}
		}, [clazz](Execution& exe, Scope& scope, Object* input, Object* name) {
			return name->adaptableTo(exe, &ClassClass::INSTANCE);
		}, [clazz](Execution& exe, Scope& scope, Object* input, Object* name, Object* value) {
			Class* otherClass = ClassClass::value(exe, name);
			
			exe.session.adaptors[clazz][otherClass] = [&scope, value, otherClass](Execution& exe, Object* from) {
				Vector<Object*> args; Map<string,Object*> flags;
				return value->execute(exe, scope, from, args, flags)->adapt(exe, otherClass);
			};
		});
	} else {
		return clazz->getStaticField(exe, name);
	}
}

bool iconus::ClassClass::canSetField(Object* self, Execution& exe,
		Object* name) {
	Class* clazz = ClassClass::value(self);
	
	if (name->equals(&CLASS_FIELD_NAME)) {
		return false;
	} else if (name->equals(&CLASS_FIELD_FIELDS)) {
		return false;
	} else if (name->equals(&CLASS_FIELD_ADAPTS)) {
		return false;
	} else {
		return clazz->canSetStaticField(exe, name);
	}
}

void iconus::ClassClass::setField(Object* self, Execution& exe, Object* name,
		Object* value) {
	Class* clazz = ClassClass::value(self);
	
	if (name->equals(&CLASS_FIELD_NAME)) {
		Class::setField(self, exe, name, value);
	} else if (name->equals(&CLASS_FIELD_FIELDS)) {
		Class::setField(self, exe, name, value);
	} else if (name->equals(&CLASS_FIELD_ADAPTS)) {
		Class::setField(self, exe, name, value);
	} else {
		clazz->setStaticField(exe, name, value);
	}
}

iconus::ClassEvent iconus::ClassEvent::INSTANCE{};

void iconus::ClassEvent::fire(Object* self, Execution& exe, Scope& scope,
		Object* input, Vector<Object*>& args,
		Map<std::string, Object*>& flags) {
	Object* conns; {
		Lock lock{self->mutex};
		conns = ClassEvent::value(self);
	}
	
	Lock lock{conns->mutex};
	for (Object* value : conns->fieldValues(exe)) {
		auto& conn = ClassEventConnection::value(value);
		conn.handler->execute(exe, scope, input, args, flags);
	}
}

Object* iconus::ClassEvent::connect(Object* event, Object* handler) {
	Lock lock{event->mutex};
	Object* conns = ClassEvent::value(event);
	Object* conn = ClassEventConnection::create(event, handler);
	
	// TODO don't assume the connections is stored as a list
	Lock lock2{conns->mutex};
	auto& list = ClassList::value(conns);
	list.emplace_back(conn);
	
	// end of TODO
	return conn;
}

std::string iconus::ClassEvent::name() {
	return "event";
}

iconus::ClassEventConnection iconus::ClassEventConnection::INSTANCE{};

void iconus::ClassEventConnection::disconnect(Object* self) {
	Lock lock{self->mutex};
	auto& conn = ClassEventConnection::value(self);
	
	// TODO don't assume the connections is stored as a list
	Object* conns = ClassEvent::value(conn.event);
	Lock lock2{conns->mutex};
	auto& list = ClassList::value(conns);
	
	// end of TODO
	auto it = find_if(list.begin(), list.end(), [&](Object* x) {
		return x->equals(self);
	});
	if (it != list.end()) {
		list.erase(it);
	}
}

std::string iconus::ClassEventConnection::name() {
	return "event-connection";
}

iconus::ClassSpecialMap iconus::ClassSpecialMap::INSTANCE{};

iconus::ClassSpecialMap::Instance::Instance(
		std::function<void(Execution&, Scope&, Object*, Deque<Object*>&)> fieldNames,
		std::function<Object*(Execution&, Scope&, Object*, Object*)> getField,
		std::function<bool(Execution&, Scope&, Object*, Object*)> canSetField,
		std::function<void(Execution&, Scope&, Object*, Object*, Object*)> setField) {
	this->fieldNames = ClassSystemFunction::create([fieldNames](Execution& exe, Scope& scope, Object* input, auto& args, auto& flags) {
		Object* result = ClassList::create();
		auto& list = ClassList::value(result);
		fieldNames(exe, scope, input, list);
		return result;
	});
	
	this->getField = ClassSystemFunction::create([getField](Execution& exe, Scope& scope, Object* input, auto& args, auto& flags) {
		return getField(exe, scope, input, args[0]);
	});
	
	this->canSetField = ClassSystemFunction::create([canSetField](Execution& exe, Scope& scope, Object* input, auto& args, auto& flags) {
		return ClassBool::create(canSetField(exe, scope, input, args[0]));
	});
	
	this->setField = ClassSystemFunction::create([setField](Execution& exe, Scope& scope, Object* input, auto& args, auto& flags) {
		setField(exe, scope, input, args[0], args[1]);
		return &ClassNil::NIL;
	});
}

std::string iconus::ClassSpecialMap::name() {
	return "map*";
}

Vector<Object*> iconus::ClassSpecialMap::fieldNames(Object* self,
		Execution& exe) {
	auto& handler = ClassSpecialMap::value(self);
	Vector<Object*> args; Map<string,Object*> flags;
	Object* result = handler.fieldNames->execute(exe, exe.session.sessionScope, self, args, flags);
	auto& list = ClassList::value(exe, result);
	return Vector<Object*>(list.begin(), list.end());
}

Object* iconus::ClassSpecialMap::getField(Object* self, Execution& exe,
		Object* name) {
	auto& handler = ClassSpecialMap::value(self);
	Vector<Object*> args{{name}}; Map<string,Object*> flags;
	return handler.getField->execute(exe, exe.session.sessionScope, self, args, flags);
}

bool iconus::ClassSpecialMap::canSetField(Object* self, Execution& exe,
		Object* name) {
	auto& handler = ClassSpecialMap::value(self);
	Vector<Object*> args{{name}}; Map<string,Object*> flags;
	return handler.canSetField->execute(exe, exe.session.sessionScope, self, args, flags)->truthy();
}

void iconus::ClassSpecialMap::setField(Object* self, Execution& exe,
		Object* name, Object* value) {
	auto& handler = ClassSpecialMap::value(self);
	Vector<Object*> args{{name, value}}; Map<string,Object*> flags;
	handler.setField->execute(exe, exe.session.sessionScope, self, args, flags);
}
