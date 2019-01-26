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
		static inline Class* value(Execution& exe, Object* ob) {
			return (Class*) ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::string toString(Object* self, Execution& exe) override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
	};
}

#endif /* PLUGINS_STDLIB_STD_CLASSES_HPP_ */
