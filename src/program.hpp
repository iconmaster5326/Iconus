/*
 * program.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_PROGRAM_HPP_
#define SRC_PROGRAM_HPP_

#include <string>
#include <vector>
#include <unordered_map>

namespace iconus {
	class Object; class Scope; class Session;
	
	class Class {
	public:
		virtual ~Class();
		virtual std::string name();
		virtual std::string toString(Object* self);
		virtual bool executable();
		virtual Object* execute(Object* self, Session& session, Scope& scope, Object* input, std::vector<Object*>& args, std::unordered_map<std::string,Object*>& flags);
		
		virtual std::vector<Object*> fieldNames(Object* self);
		virtual bool hasField(Object* self, Object* name);
		virtual Object* getField(Object* self, Object* name);
		virtual bool canSetField(Object* self, Object* name);
		virtual void setField(Object* self, Object* name, Object* value);
	};
	
	class Object {
	public:
		inline Object(Class* clazz) : clazz(clazz) {}
		inline Object(Class* clazz, double value) : clazz(clazz), value{.asDouble = value} {}
		inline Object(Class* clazz, void* value) : clazz(clazz), value{.asPtr = value} {}
		
		static Object* castTo(Object* ob, Class* clazz);
		
		inline operator std::string() {
			return clazz->toString(this);
		}
		inline bool executable() {
			return clazz->executable();
		}
		inline Object* execute(Session& session, Scope& scope, Object* input, std::vector<Object*>& args, std::unordered_map<std::string,Object*>& flags) {
			return clazz->execute(this, session, scope, input, args, flags);
		}
		
		inline std::vector<Object*> fieldNames() {
			return clazz->fieldNames(this);
		}
		inline bool hasField(Object* name) {
			return clazz->hasField(this, name);
		}
		inline Object* getField(Object* name) {
			return clazz->getField(this, name);
		}
		inline bool canSetField(Object* name) {
			return clazz->canSetField(this, name);
		}
		inline void setField(Object* name, Object* value) {
			clazz->setField(this, name, value);
		}
		
		Class* clazz;
		union {
			double asDouble;
			void* asPtr;
		} value;
	};
	
	class Scope {
	public:
		std::unordered_map<std::string, Object*> vars;
		Scope* parent;
		Object* input;
		
		Scope();
		Scope(Scope* parent);
		Scope(Scope* parent, Object* input);
		
		Object* get(const std::string& name);
		void set(const std::string& name, Object* value);
		void setLocal(const std::string& name, Object* value);
	};
}

#endif /* SRC_PROGRAM_HPP_ */
