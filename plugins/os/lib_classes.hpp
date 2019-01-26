/*
 * classes.hpp
 *
 *  Created on: Jan 25, 2019
 *      Author: iconmaster
 */

#ifndef PLUGINS_FILES_LIB_CLASSES_HPP_
#define PLUGINS_FILES_LIB_CLASSES_HPP_

#include "classes.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace iconus {
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
			
			bool done;
			int retCode;
			Vector<Line> lines;
			boost::uuids::uuid id;
			
			inline Instance() : retCode(0), done(false), id(boost::uuids::random_generator()()) {}
			template<typename... Args> inline Instance(int retCode, Args... args) : done(true), retCode(retCode), lines(args...), id(boost::uuids::random_generator()()) {}
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
		
		Vector<Object*> fieldNames(Object* self, Execution& exe) override;
		Object* getField(Object* self, Execution& exe, Object* name) override;
	};
}

#endif /* PLUGINS_FILES_LIB_CLASSES_HPP_ */
