/*
 * classes.cpp
 *
 *  Created on: Jan 23, 2019
 *      Author: iconmaster
 */


#include "std_classes.hpp"
#include "error.hpp"
#include "session.hpp"

#include <sstream>

using namespace std;
using namespace iconus;

iconus::ClassClass iconus::ClassClass::INSTANCE{};

std::string iconus::ClassClass::name() {
	return "class";
}

std::string iconus::ClassClass::toString(Object* self, Session& session) {
	Class* value = (Class*) self->value.asPtr;
	return "class "+value->name();
}

std::size_t iconus::ClassClass::hash(const Object* self) const {
	return (size_t) self->value.asPtr;
}

bool iconus::ClassClass::equals(const Object* self, const Object* other) const {
	return self->value.asPtr == other->value.asPtr;
}

iconus::ClassImage iconus::ClassImage::INSTANCE{};

std::string iconus::ClassImage::name() {
	return "image";
}

std::string iconus::ClassImage::toString(Object* self, Session& session) {
	return "(image...)";
}

std::size_t iconus::ClassImage::hash(const Object* self) const {
	return std::hash<string>()(*(string*)self->value.asPtr);
}

bool iconus::ClassImage::equals(const Object* self, const Object* other) const {
	return self->value.asPtr == other->value.asPtr;
}

iconus::ClassRawString iconus::ClassRawString::INSTANCE{};

std::string iconus::ClassRawString::name() {
	return "raw-string";
}
