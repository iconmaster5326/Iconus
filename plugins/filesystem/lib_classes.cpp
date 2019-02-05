/*
 * classes.cpp
 *
 *  Created on: Jan 25, 2019
 *      Author: iconmaster
 */


#include "lib_classes.hpp"

#include "error.hpp"
#include "session.hpp"

#include <sstream>

using namespace std;
using namespace iconus;
using namespace boost::filesystem;

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

iconus::ClassFile iconus::ClassFile::INSTANCE{};

std::string iconus::ClassFile::name() {
	return "file";
}

std::size_t iconus::ClassFile::hash(const Object* self) const {
	path& value = ClassFile::value(self);
	return std::hash<string>()(value.string());
}

bool iconus::ClassFile::equals(const Object* self, const Object* other) const {
	path& a = ClassFile::value(self);
	path& b = ClassFile::value(other);
	
	return a == b;
}
