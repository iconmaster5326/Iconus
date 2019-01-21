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
	class Object; class Scope; class Session;
	
	class Class : public gc {
	public:
		virtual ~Class();
		virtual std::string name();
		virtual std::string toString(Object* self, Session& session);
		
		// C++ properties; session is not available due to use in std::hash, etc.
		virtual std::size_t hash(const Object* self) const;
		virtual bool equals(const Object* self, const Object* other) const;
		
		// execution
		virtual bool executable(Object* self, Session& session);
		virtual Object* execute(Object* self, Session& session, Scope& scope, Object* input, Vector<Object*>& args, Map<std::string,Object*>& flags);
		
		// necesary to implement fields
		virtual Vector<Object*> fieldNames(Object* self, Session& session);
		virtual bool hasField(Object* self, Session& session, Object* name);
		virtual Object* getField(Object* self, Session& session, Object* name);
		virtual bool canSetField(Object* self, Session& session, Object* name);
		virtual void setField(Object* self, Session& session, Object* name, Object* value);
		
		// optional to implement fields
		virtual Vector<Object*> fieldValues(Object* self, Session& session);
		virtual Vector<std::pair<Object*,Object*> > fields(Object* self, Session& session);
	};
	
	class Object : public gc {
	public:
		inline Object(Class* clazz) : clazz(clazz) {}
		inline Object(Class* clazz, double value) : clazz(clazz), value{.asDouble = value} {}
		inline Object(Class* clazz, void* value) : clazz(clazz), value{.asPtr = value} {}
		
		inline std::string toString(Session& session) {
			return clazz->toString(this, session);
		}
		
		inline std::size_t hash() const {
			return clazz->hash(this);
		}
		inline bool equals(Object* other) const {
			return clazz->equals(this, other);
		}
		
		inline bool executable(Session& session)  {
			return clazz->executable(this, session);
		}
		inline Object* execute(Session& session, Scope& scope, Object* input, Vector<Object*>& args, Map<std::string,Object*>& flags) {
			return clazz->execute(this, session, scope, input, args, flags);
		}
		
		inline Vector<Object*> fieldNames(Session& session) {
			return clazz->fieldNames(this, session);
		}
		inline Vector<Object*> fieldValues(Session& session) {
			return clazz->fieldValues(this, session);
		}
		inline Vector<std::pair<Object*,Object*> > fields(Session& session) {
			return clazz->fields(this, session);
		}
		inline bool hasField(Session& session, Object* name) {
			return clazz->hasField(this, session, name);
		}
		inline Object* getField(Session& session, Object* name) {
			return clazz->getField(this, session, name);
		}
		inline bool canSetField(Session& session, Object* name) {
			return clazz->canSetField(this, session, name);
		}
		inline void setField(Session& session, Object* name, Object* value) {
			clazz->setField(this, session, name, value);
		}
		
		bool adaptableTo(Session& session, Class* clazz);
		Object* adapt(Session& session, Class* clazz);
		
		Class* clazz;
		union {
			double asDouble;
			void* asPtr;
		} value;
	};
	
	class Scope : public gc {
	public:
		Map<std::string, Object*> vars;
		Scope* parent;
		Object* input;
		
		Scope();
		Scope(Scope* parent);
		Scope(Scope* parent, Object* input);
		virtual ~Scope();
		
		virtual Object* get(const std::string& name);
		virtual void set(const std::string& name, Object* value);
		virtual void setLocal(const std::string& name, Object* value);
		virtual bool canSet(const std::string& name);
	};
}

template<> struct std::hash<iconus::Object*> {
	size_t operator()(const iconus::Object* ob) const {
		return ob->hash();
	};
};

template<> struct std::equal_to<iconus::Object*> {
	constexpr bool operator()(const iconus::Object* a, const iconus::Object* b) const {
		return a->value.asPtr == b->value.asPtr;
	};
};

#endif /* SRC_PROGRAM_HPP_ */
