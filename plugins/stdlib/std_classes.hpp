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
	
	class ClassSystemOutput : public Class {
	public:
		class Instance : public gc {
		public:
			class Line : public gc {
			public:
				bool isErr;
				std::string text;
				
				inline Line(bool isErr, const std::string& text) : isErr(isErr), text(text) {}
			};
			
			int retCode;
			Vector<Line> lines;
			
			inline Instance() : retCode(0) {}
			template<typename... Args> inline Instance(int retCode, Args... args) : retCode(retCode), lines(args...) {}
		};
		
		static ClassSystemOutput INSTANCE;
		static inline Object* create(Instance* i) {
			return new Object(&INSTANCE, i);
		}
		static inline Object* create(const Instance& i) {
			return new Object(&INSTANCE, new Instance(i));
		}
		template<typename... Args> static inline Object* create(Args... args) {
			return new Object(&INSTANCE, new Instance(args...));
		}
		
		static inline Instance& value(Execution& exe, Object* ob) {
			return *(Instance*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
	};
}

#endif /* PLUGINS_STDLIB_STD_CLASSES_HPP_ */
