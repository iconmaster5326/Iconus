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

iconus::Error::Error(const std::string& value) : whatString(value), value(new Object(&ClassString::INSTANCE, new string(value))) {}
iconus::Error::Error(Object* value) : whatString(value->operator string()), value(value) {}

const char* iconus::Error::what() const noexcept {
	return whatString.c_str();
}