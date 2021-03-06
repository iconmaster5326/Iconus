/*
 * error.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "error.hpp"
#include "program.hpp"
#include "classes.hpp"

using namespace std;
using namespace iconus;

Source iconus::Source::UNKNOWN{};

iconus::Error::Error(const std::string& value) : whatString(value), value(ClassString::create(value)), stackTrace{StackTrace::callStack} {}
iconus::Error::Error(Execution& exe, Object* value) : whatString(value->toString(exe)), value(value), stackTrace{StackTrace::callStack} {}
iconus::Error::Error(const std::string& value, Vector<StackTrace>& stackTrace) : whatString(value), value(new Object(&ClassError::INSTANCE, ClassString::create(value))), stackTrace{stackTrace} {}
iconus::Error::Error(Execution& exe, Object* value, Vector<StackTrace>& stackTrace) : whatString(value->toString(exe)), value(value), stackTrace{stackTrace} {}

const char* iconus::Error::what() const noexcept {
	return whatString.c_str();
}

thread_local Vector<StackTrace> iconus::StackTrace::callStack;

void iconus::StackTrace::enter(Type type, const Source& source, const std::string& name) {
	callStack.emplace_back(type, source, name);
}

void iconus::StackTrace::exit() {
	callStack.pop_back();
}
