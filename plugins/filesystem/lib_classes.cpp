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
	if (self) {
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
	} else {
		return Vector<Object*>({
			&FILE_FIELD_NAME, &FILE_FIELD_PARENT, &FILE_FIELD_TYPE, 
			&FILE_FIELD_LINK, &FILE_FIELD_PERM, &FILE_FIELD_USER,
			&FILE_FIELD_GROUP, &FILE_FIELD_MTIME, &FILE_FIELD_SIZE
		});
	}
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

DEF_FIELD("user-read?", PERMS_FIELD_UR);
DEF_FIELD("user-write?", PERMS_FIELD_UW);
DEF_FIELD("user-run?", PERMS_FIELD_UX);
DEF_FIELD("group-read?", PERMS_FIELD_GR);
DEF_FIELD("group-write?", PERMS_FIELD_GW);
DEF_FIELD("group-run?", PERMS_FIELD_GX);
DEF_FIELD("all-read?", PERMS_FIELD_OR);
DEF_FIELD("all-write?", PERMS_FIELD_OW);
DEF_FIELD("all-run?", PERMS_FIELD_OX);
static Vector<Object*> PERMS_FIELDS{{
	&PERMS_FIELD_UR, &PERMS_FIELD_UW, &PERMS_FIELD_UX,
	&PERMS_FIELD_GR, &PERMS_FIELD_GW, &PERMS_FIELD_GX,
	&PERMS_FIELD_OR, &PERMS_FIELD_OW, &PERMS_FIELD_OX
}};

Vector<Object*> iconus::ClassPerms::fieldNames(Object* self, Execution& exe) {
	return PERMS_FIELDS;
}

Object* iconus::ClassPerms::getField(Object* self, Execution& exe,
		Object* name) {
	perms p = ClassPerms::value(self);
	
	if (name->equals(&PERMS_FIELD_UR)) {
		return ClassBool::create(p & perms::owner_read);
	} else if (name->equals(&PERMS_FIELD_UW)) {
		return ClassBool::create(p & perms::owner_write);
	} else if (name->equals(&PERMS_FIELD_UX)) {
		return ClassBool::create(p & perms::owner_exe);
	} else if (name->equals(&PERMS_FIELD_GR)) {
		return ClassBool::create(p & perms::group_read);
	} else if (name->equals(&PERMS_FIELD_GW)) {
		return ClassBool::create(p & perms::group_write);
	} else if (name->equals(&PERMS_FIELD_GX)) {
		return ClassBool::create(p & perms::group_exe);
	} else if (name->equals(&PERMS_FIELD_OR)) {
		return ClassBool::create(p & perms::others_read);
	} else if (name->equals(&PERMS_FIELD_OW)) {
		return ClassBool::create(p & perms::others_write);
	} else if (name->equals(&PERMS_FIELD_OX)) {
		return ClassBool::create(p & perms::others_exe);
	} else {
		return Class::getField(self, exe, name);
	}
}
