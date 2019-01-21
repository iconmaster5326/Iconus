/*
 * classes.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_CLASSES_HPP_
#define SRC_CLASSES_HPP_

#include "program.hpp"
#include "op.hpp"
#include "function.hpp"

#include <functional>

#include <initializer_list>


namespace iconus {
	class ClassNil : public Class {
	public:
		static ClassNil INSTANCE;
		static Object NIL;
		std::string name() override;
		std::string toString(Object* self, Session& session) override;
	};
	
	class ClassBool : public Class {
	public:
		static ClassBool INSTANCE;
		static Object TRUE, FALSE;
		std::string name() override;
		std::string toString(Object* self, Session& session) override;
	};
	
	class ClassNumber : public Class {
	public:
		static ClassNumber INSTANCE;
		static inline Object* create(double n) {
			return new Object(&INSTANCE, n);
		}
		static inline double& value(Session& session, Object* ob) {
			return ob->adapt(session, &INSTANCE)->value.asDouble;
		}
		
		std::string name() override;
		std::string toString(Object* self, Session& session) override;
	};
	
	class ClassString : public Class {
	public:
		static ClassString INSTANCE;
		static inline Object* create(const std::string& s) {
			return new Object(&INSTANCE, gcAlloc<std::string>(s));
		}
		static inline std::string& value(Session& session, Object* ob) {
			return *(std::string*)ob->adapt(session, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::string toString(Object* self, Session& session) override;
	};
	
	class ClassSystemFunction : public Class {
	public:
		using Handler = std::function<Object*(Session&, Scope&, Object*, Vector<Object*>&, Map<std::string,Object*>&)>;
		
		static ClassSystemFunction INSTANCE;
		std::string name() override;
		bool executable(Object* self, Session& session) override;
		Object* execute(Object* self, Session& session, Scope& scope, Object* input, Vector<Object*>& args, Map<std::string,Object*>& flags) override;
	};
	
	class ClassManagedFunction : public ClassSystemFunction {
	public:
		using Handler = std::function<Object*(Session&, Scope&, Object*, Map<std::string,Object*>&, Vector<Object*>&, Map<std::string,Object*>&)>;
		
		class Instance : public gc {
		public:
			inline Instance(const Function& fn, Handler handler) : handler(handler), fn(fn) {}
			
			inline Instance(std::string input, std::string vararg, std::string varflag, const Vector<Function::Arg>& args, const Vector<Function::Arg>& flags, Handler handler) :
				handler(handler), fn(input, vararg, varflag, args, flags)
			{}
			
			inline Instance(std::string input, std::string vararg, std::string varflag, std::initializer_list<Function::Arg> args, std::initializer_list<Function::Arg> flags, Handler handler) :
				handler(handler), fn(input, vararg, varflag, args, flags)
			{}
			
			Handler handler;
			Function fn;
		};
		
		static ClassManagedFunction INSTANCE;
		Object* execute(Object* self, Session& session, Scope& scope, Object* input, Vector<Object*>& args, Map<std::string,Object*>& flags) override;
	};
	
	class ClassUserFunction : public ClassManagedFunction {
	public:
		static ClassUserFunction INSTANCE;
		static Object* create(Scope& scope, Op* op, const Function& fn);
	};
	
	class ClassList : public Class {
	public:
		static ClassList INSTANCE;
		static inline Object* create(Deque<Object*>* args) {
			return new Object(&INSTANCE, args);
		}
		template<typename T> static Object* create(T args) {
			return create(gcAlloc<Deque<Object*>>(args));
		}
		template<typename T> static Object* create(T begin, T end) {
			return create(gcAlloc<Deque<Object*>>(begin, end));
		}
		static inline Deque<Object*>& value(Session& session, Object* ob) {
			return *(Deque<Object*>*)ob->adapt(session, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::string toString(Object* self, Session& session) override;
		
		Vector<Object*> fieldNames(Object* self, Session& session) override;
		bool hasField(Object* self, Session& session, Object* name) override;
		Object* getField(Object* self, Session& session, Object* name) override;
		bool canSetField(Object* self, Session& session, Object* name) override;
		void setField(Object* self, Session& session, Object* name, Object* value) override;
	};
	
	class ClassError : public Class {
	public:
		static ClassError INSTANCE;
		std::string name() override;
		std::string toString(Object* self, Session& session) override;
	};
}

#endif /* SRC_CLASSES_HPP_ */
