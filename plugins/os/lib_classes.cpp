/*
 * classes.cpp
 *
 *  Created on: Jan 25, 2019
 *      Author: iconmaster
 */


#include "../os/lib_classes.hpp"

#include "error.hpp"
#include "session.hpp"

#include <sstream>

using namespace std;
using namespace iconus;

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
