/*
 * classes.hpp
 *
 *  Created on: Feb 13, 2019
 *      Author: iconmaster
 */

#ifndef PLUGINS_GUI_LIB_CLASSES_HPP_
#define PLUGINS_GUI_LIB_CLASSES_HPP_

#include "classes.hpp"

namespace iconus {
	class ClassHBox : public Class {
	public:
		class Instance {
		public:
			Object* items;
			std::string style;
			
			inline Instance(Object* items, std::string style) : items{items}, style{style} {}
		};
		
		static ClassHBox INSTANCE;
		static inline Object* create(Instance* i) {
			return new Object(&INSTANCE, i);
		}
		template<typename... Args> static inline Object* create(Args... args) {
			return new Object(&INSTANCE, new Instance(args...));
		}
		static inline Instance& value(const Object* ob) {
			return *(Instance*)ob->value.asPtr;
		}
		static inline Instance& value(Execution& exe, Object* ob) {
			return *(Instance*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
	};

	class ClassVBox : public Class {
	public:
		class Instance {
		public:
			Object* items;
			std::string style;
			
			inline Instance(Object* items, std::string style) : items{items}, style{style} {}
		};
		
		static ClassVBox INSTANCE;
		static inline Object* create(Instance* i) {
			return new Object(&INSTANCE, i);
		}
		template<typename... Args> static inline Object* create(Args... args) {
			return new Object(&INSTANCE, new Instance(args...));
		}
		static inline Instance& value(const Object* ob) {
			return *(Instance*)ob->value.asPtr;
		}
		static inline Instance& value(Execution& exe, Object* ob) {
			return *(Instance*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
	};
}

#endif /* PLUGINS_GUI_LIB_CLASSES_HPP_ */
