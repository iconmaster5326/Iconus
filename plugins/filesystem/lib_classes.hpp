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
			return new Object(&INSTANCE, gcAlloc<std::string>(s));
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
			return new Object(&INSTANCE, gcAlloc<std::string>(s));
		}
		static inline std::string& value(Execution& exe, Object* ob) {
			return *(std::string*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
	};
	
	class ClassFile : public Class {
	public:
		static ClassFile INSTANCE;
		static inline Object* create(const std::string& s) {
			return new Object(&INSTANCE, gcAlloc<boost::filesystem::path>(boost::filesystem::absolute(boost::filesystem::path(s))));
		}
		static inline Object* create(const boost::filesystem::path& s) {
			return new Object(&INSTANCE, gcAlloc<boost::filesystem::path>(boost::filesystem::absolute(s)));
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
	};
}

#endif /* PLUGINS_FILESYSTEM_LIB_CLASSES_HPP_ */
