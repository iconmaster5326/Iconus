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
		
		// adaptors
		using Adaptor = std::function<Object*(Session&, Object*)>;
		Map<Class*, Adaptor> adaptors;
	};
	
	class Object : public gc {
	public:
		inline Object(Class* clazz) : clazz(clazz) {}
		inline Object(Class* clazz, double value) : clazz(clazz), value{.asDouble = value} {}
		inline Object(Class* clazz, void* value) : clazz(clazz), value{.asPtr = value} {}
		
		inline std::string toString(Session& session) {
			return clazz->toString(this, session);
		}
		inline bool executable(Session& session) {
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
		
		Object* get(const std::string& name);
		void set(const std::string& name, Object* value);
		void setLocal(const std::string& name, Object* value);
	};
}

#endif /* SRC_PROGRAM_HPP_ */
