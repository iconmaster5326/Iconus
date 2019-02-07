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

iconus::Error::Error(const std::string& value) : whatString(value), value(ClassString::create(value)), stackTrace{StackTrace::callStack} {}
iconus::Error::Error(Execution& exe, Object* value) : whatString(value->toString(exe)), value(value), stackTrace{StackTrace::callStack} {}
iconus::Error::Error(const std::string& value, Vector<StackTrace>& stackTrace) : whatString(value), value(new Object(&ClassError::INSTANCE, ClassString::create(value))), stackTrace{stackTrace} {}
iconus::Error::Error(Execution& exe, Object* value, Vector<StackTrace>& stackTrace) : whatString(value->toString(exe)), value(value), stackTrace{stackTrace} {}

const char* iconus::Error::what() const noexcept {
	return whatString.c_str();
}

thread_local Vector<StackTrace> iconus::StackTrace::callStack;

void iconus::StackTrace::enter(const std::string& name, const std::string& file, int line) {
	callStack.emplace_back(name, file, line);
}

void iconus::StackTrace::exit() {
	callStack.pop_back();
}
