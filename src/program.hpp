/*
 * program.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_PROGRAM_HPP_
#define SRC_PROGRAM_HPP_

#include <string>
#include <functional>

#include "gc.hpp"

namespace iconus {
	class Object; class Scope; class Session; class Execution;
	
	class Class : public gc {
	public:
		virtual ~Class();
		virtual std::string name();
		
		// C++ properties; session is not available due to use in std::hash, etc.
		virtual std::size_t hash(const Object* self) const;
		virtual bool equals(const Object* self, const Object* other) const;
		
		// execution
		virtual bool executable(Object* self, Execution& exe);
		virtual Object* execute(Object* self, Execution& exe, Scope& scope, Object* input, Vector<Object*>& args, Map<std::string,Object*>& flags);
		
		// necesary to implement fields
		virtual Vector<Object*> fieldNames(Object* self, Execution& exe);
		virtual Object* getField(Object* self, Execution& exe, Object* name);
		virtual bool canSetField(Object* self, Execution& exe, Object* name);
		virtual void setField(Object* self, Execution& exe, Object* name, Object* value);
		
		// optional to implement fields
		virtual bool hasField(Object* self, Execution& exe, Object* name);
		virtual Vector<Object*> fieldValues(Object* self, Execution& exe);
		virtual Vector<std::pair<Object*,Object*> > fields(Object* self, Execution& exe);
	};
	
	class Object : public gc {
	public:
		inline Object(Class* clazz) : clazz(clazz) {}
		inline Object(Class* clazz, double value) : clazz(clazz), value{.asDouble = value} {}
		inline Object(Class* clazz, void* value) : clazz(clazz), value{.asPtr = value} {}
		
		bool truthy();
		
		inline std::size_t hash() const {
			return clazz->hash(this);
		}
		inline bool equals(Object* other) const {
			return clazz->equals(this, other);
		}
		
		inline bool executable(Execution& exe)  {
			return clazz->executable(this, exe);
		}
		inline Object* execute(Execution& exe, Scope& scope, Object* input, Vector<Object*>& args, Map<std::string,Object*>& flags) {
			return clazz->execute(this, exe, scope, input, args, flags);
		}
		
		inline Vector<Object*> fieldNames(Execution& exe) {
			return clazz->fieldNames(this, exe);
		}
		inline Vector<Object*> fieldValues(Execution& exe) {
			return clazz->fieldValues(this, exe);
		}
		inline Vector<std::pair<Object*,Object*> > fields(Execution& exe) {
			return clazz->fields(this, exe);
		}
		inline bool hasField(Execution& exe, Object* name) {
			return clazz->hasField(this, exe, name);
		}
		inline Object* getField(Execution& exe, Object* name) {
			return clazz->getField(this, exe, name);
		}
		inline bool canSetField(Execution& exe, Object* name) {
			return clazz->canSetField(this, exe, name);
		}
		inline void setField(Execution& exe, Object* name, Object* value) {
			clazz->setField(this, exe, name, value);
		}
		
		bool adaptableTo(Execution& exe, Class* clazz);
		Object* adapt(Execution& exe, Class* clazz);
		std::string toString(Execution& exe);
		
		template<typename T> inline T get() {
			Lock lock{mutex};
			return (T) value.asPtr;
		}
		template<typename T> inline void set(T t) {
			Lock lock{mutex};
			value.asPtr = (void*) t;
		}
		template<typename Fn> inline void update(Fn fn) {
			Lock lock{mutex};
			value.asPtr = (void*) fn(value.asPtr);
		}
		
		Mutex mutex;
		Class* clazz;
		union {
			double asDouble;
			void* asPtr;
		} value;
	};
	
	template<> inline double Object::get<double>() {
		return value.asDouble;
	}
	template<> inline void Object::set<double>(double d) {
		value.asDouble = d;
	}
	
	class Scope : public gc {
	public:
		Map<std::string, Object*> vars;
		Scope* parent;
		Object* input;
		Mutex mutex;
		
		Scope();
		Scope(Scope* parent);
		Scope(Scope* parent, Object* input);
		Scope(const Scope&) = delete;
		virtual ~Scope();
		
		virtual Object* get(const std::string& name);
		virtual void set(const std::string& name, Object* value);
		virtual void setLocal(const std::string& name, Object* value);
		virtual bool canSet(const std::string& name);
		
		// addMethod intended for plugin writers and not users!
		void addMethod(const std::string& name, Object* predicate, Object* handler);
		void addMethod(const std::string& name, Object* handler);
	};
}

template<> struct std::hash<iconus::Object*> {
	size_t operator()(const iconus::Object* ob) const {
		return ob->hash();
	};
};

template<> struct std::equal_to<iconus::Object*> {
	constexpr bool operator()(const iconus::Object* a, const iconus::Object* b) const {
		return a->equals((iconus::Object*)b);
	};
};

#endif /* SRC_PROGRAM_HPP_ */
