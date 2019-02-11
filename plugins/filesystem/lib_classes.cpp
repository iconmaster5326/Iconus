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
#include <sys/stat.h>

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
	return "str/raw";
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

iconus::ClassPerms iconus::ClassPerms::INSTANCE{};

std::string iconus::ClassPerms::name() {
	return "perms";
}

std::size_t iconus::ClassPerms::hash(const Object* self) const {
	return (std::size_t) ClassPerms::value(self);
}

bool iconus::ClassPerms::equals(const Object* self, const Object* other) const {
	return ClassPerms::value(self) == ClassPerms::value(other);
}

#define DEF_FIELD(str,name) static string name##_S{str}; static Object name{&ClassString::INSTANCE, &name##_S}

DEF_FIELD("type", FILE_FIELD_TYPE);
DEF_FIELD("perms", FILE_FIELD_PERM);
DEF_FIELD("hard-links", FILE_FIELD_LINK);
DEF_FIELD("user", FILE_FIELD_USER);
DEF_FIELD("group", FILE_FIELD_GROUP);
DEF_FIELD("size", FILE_FIELD_SIZE);
DEF_FIELD("last-changed", FILE_FIELD_MTIME);
DEF_FIELD("name", FILE_FIELD_NAME);
DEF_FIELD("..", FILE_FIELD_PARENT);

Vector<Object*> iconus::ClassFile::fieldNames(Object* self, Execution& exe) {
	Vector<Object*> v{{&FILE_FIELD_NAME, &FILE_FIELD_PARENT}};
	path& p = ClassFile::value(self);
	if (exists(p)) {
		v.push_back(&FILE_FIELD_TYPE);
		v.push_back(&FILE_FIELD_LINK);
		v.push_back(&FILE_FIELD_PERM);
		v.push_back(&FILE_FIELD_USER);
		v.push_back(&FILE_FIELD_GROUP);
		v.push_back(&FILE_FIELD_MTIME);
	}
	if (is_regular_file(p)) {
		v.push_back(&FILE_FIELD_SIZE);
	}
	return v;
}

Object* iconus::ClassFile::getField(Object* self, Execution& exe,
		Object* name) {
	path& p = ClassFile::value(self);
	
	if (name->equals(&FILE_FIELD_TYPE)) {
		return ClassFile::fileType(p);
	} else if (name->equals(&FILE_FIELD_SIZE) && is_regular_file(p)) {
		return  ClassNumber::create((double) file_size(p));
	} else if (name->equals(&FILE_FIELD_PERM) && exists(p)) {
		return ClassPerms::create(status(p).permissions());
	} else if (name->equals(&FILE_FIELD_LINK) && exists(p)) {
		return ClassNumber::create((double) hard_link_count(p));
	} else if (name->equals(&FILE_FIELD_USER) && exists(p)) {
		struct stat stats;
		stat(p.string().c_str(), &stats);
		
		return ClassString::create(User::uidToString(stats.st_uid));
	} else if (name->equals(&FILE_FIELD_GROUP) && exists(p)) {
		struct stat stats;
		stat(p.string().c_str(), &stats);
		
		return ClassString::create(User::gidToString(stats.st_gid));
	} else if (name->equals(&FILE_FIELD_MTIME) && exists(p)) {
		return ClassTime::create(last_write_time(p));
	} else if (name->equals(&FILE_FIELD_NAME)) {
		return ClassString::create(p.filename().string());
	} else if (name->equals(&FILE_FIELD_PARENT)) {
		path temp = p;
		if (temp.parent_path() != path()) temp.remove_filename();
		return ClassFile::create(temp);
	} else {
		return Class::getField(self, exe, name);
	}
}

bool iconus::ClassFile::canSetField(Object* self, Execution& exe,
		Object* name) {
	return false;
}

void iconus::ClassFile::setField(Object* self, Execution& exe, Object* name,
		Object* value) {
	Class::setField(self, exe, name, value);
}

Object* iconus::ClassFile::fileType(boost::filesystem::path& p)  {
	switch (status(p).type()) {
	case file_type::file_not_found: return &ClassNil::NIL;
	case file_type::directory_file: return ClassString::create("dir");
	case file_type::regular_file: return ClassString::create("file");
	case file_type::symlink_file: return ClassString::create("link");
	case file_type::block_file: return ClassString::create("block");
	case file_type::character_file: return ClassString::create("char");
	case file_type::fifo_file: return ClassString::create("fifo");
	case file_type::socket_file: return ClassString::create("socket");
	default: return ClassString::create("?");
	}
}

boost::filesystem::path iconus::ClassFile::makeAbsolute(boost::filesystem::path p, boost::filesystem::path base) {
	if (!exists(p)) {
		path fname = p.filename();
		if (fname == path(".")) {
			return makeAbsolute(absolute(p, base).remove_filename(), base);
		} else if (fname == path("..")) {
			return makeAbsolute(absolute(p, base).remove_filename(), base).remove_filename();
		} else {
			return makeAbsolute(absolute(p, base).remove_filename(), base) / fname;
		}
	} else {
		return canonical(p, base);
	}
}
