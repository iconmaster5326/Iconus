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

iconus::ClassSystemOutput iconus::ClassSystemOutput::INSTANCE{};

std::string iconus::ClassSystemOutput::name() {
	return "system-output";
}

std::size_t iconus::ClassSystemOutput::hash(const Object* self) const {
	Instance& a = *(Instance*)self->value.asPtr;
	size_t hash = ((unsigned)a.retCode) + 1;
	for (const auto& line : a.lines) {
		hash *= (line.isErr ? -1 : 1) * std::hash<string>()(line.text); 
	}
	return hash;
}

bool iconus::ClassSystemOutput::equals(const Object* self,
		const Object* other) const {
	Instance& a = *(Instance*)self->value.asPtr;
	Instance& b = *(Instance*)other->value.asPtr;
	
	if (a.retCode != b.retCode) return false;
	if (a.lines.size() != b.lines.size()) return false;
	for (int i = 0; i < a.lines.size(); i++) {
		if (a.lines[i].isErr != b.lines[i].isErr) return false;
		if (a.lines[i].text != b.lines[i].text) return false;
	}
	
	return true;
}
