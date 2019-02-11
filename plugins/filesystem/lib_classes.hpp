/*
 * classes.hpp
 *
 *  Created on: Jan 25, 2019
 *      Author: iconmaster
 */

#ifndef PLUGINS_FILESYSTEM_LIB_CLASSES_HPP_
#define PLUGINS_FILESYSTEM_LIB_CLASSES_HPP_

#include "classes.hpp"

#include <boost/filesystem.hpp>

namespace iconus {
	class ClassImage : public Class {
	public:
		static ClassImage INSTANCE;
		static inline Object* create(const std::string& s) {
			return new Object(&INSTANCE, new std::string(s));
		}
		static inline std::string& value(Execution& exe, Object* ob) {
			return *(std::string*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
	};
	
	class ClassRawString : public ClassString {
	public:
		static ClassRawString INSTANCE;
		static inline Object* create(const std::string& s) {
			return new Object(&INSTANCE, new std::string(s));
		}
		static inline std::string& value(Execution& exe, Object* ob) {
			return *(std::string*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
	};
	
	class ClassFile : public Class {
	public:
		static Object* fileType(boost::filesystem::path& p);
		static boost::filesystem::path makeAbsolute(boost::filesystem::path p, boost::filesystem::path base = boost::filesystem::current_path());
		
		static ClassFile INSTANCE;
		static inline Object* create(const std::string& s) {
			return new Object(&INSTANCE, new boost::filesystem::path(makeAbsolute(boost::filesystem::path(s))));
		}
		static inline Object* create(const boost::filesystem::path& s) {
			return new Object(&INSTANCE, new boost::filesystem::path(makeAbsolute(s)));
		}
		static inline boost::filesystem::path& value(const Object* ob) {
			return *(boost::filesystem::path*)ob->value.asPtr;
		}
		static inline boost::filesystem::path& value(Execution& exe, Object* ob) {
			return *(boost::filesystem::path*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
		
		Vector<Object*> fieldNames(Object* self, Execution& exe) override;
		Object* getField(Object* self, Execution& exe, Object* name) override;
		bool canSetField(Object* self, Execution& exe, Object* name) override;
		void setField(Object* self, Execution& exe, Object* name, Object* value) override;
	};
	
	class ClassPerms : public Class {
	public:
		static ClassPerms INSTANCE;
		static inline Object* create(boost::filesystem::perms perms) {
			return new Object(&INSTANCE, (uint64_t) perms);
		}
		static inline boost::filesystem::perms value(const Object* ob) {
			return (boost::filesystem::perms) ob->value.asInt;
		}
		static inline boost::filesystem::perms value(Execution& exe, Object* ob) {
			return (boost::filesystem::perms) ob->adapt(exe, &INSTANCE)->value.asInt;
		}
		
		std::string name() override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
	};
}

#endif /* PLUGINS_FILESYSTEM_LIB_CLASSES_HPP_ */
