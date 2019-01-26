/*
 * classes.hpp
 *
 *  Created on: Jan 25, 2019
 *      Author: iconmaster
 */

#ifndef PLUGINS_FILES_LIB_CLASSES_HPP_
#define PLUGINS_FILES_LIB_CLASSES_HPP_

#include "classes.hpp"

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
}

#endif /* PLUGINS_FILES_LIB_CLASSES_HPP_ */
