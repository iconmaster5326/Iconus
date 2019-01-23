/*
 * classes.hpp
 *
 *  Created on: Jan 23, 2019
 *      Author: iconmaster
 */

#ifndef PLUGINS_STDLIB_STD_CLASSES_HPP_
#define PLUGINS_STDLIB_STD_CLASSES_HPP_

#include "classes.hpp"

namespace iconus {
	class ClassClass : public Class {
	public:
		static ClassClass INSTANCE;
		static inline Object* create(Class* clazz) {
			return new Object(&INSTANCE, clazz);
		}
		static inline Class* value(Session& session, Object* ob) {
			return (Class*) ob->adapt(session, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::string toString(Object* self, Session& session) override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
	};
	
	class ClassImage : public Class {
	public:
		static ClassImage INSTANCE;
		static inline Object* create(const std::string& s) {
			return new Object(&INSTANCE, gcAlloc<std::string>(s));
		}
		static inline std::string& value(Session& session, Object* ob) {
			return *(std::string*)ob->adapt(session, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::string toString(Object* self, Session& session) override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
	};
}

#endif /* PLUGINS_STDLIB_STD_CLASSES_HPP_ */
