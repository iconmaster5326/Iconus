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

iconus::Error::Error(const std::string& value) : whatString(value), value(new Object(&ClassError::INSTANCE, ClassString::create(value))) {}
iconus::Error::Error(Object* value) : whatString("TODO"), value(value) {} // TODO

const char* iconus::Error::what() const noexcept {
	return whatString.c_str();
}
