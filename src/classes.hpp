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

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <initializer_list>

namespace iconus {
	class ClassNil : public Class {
	public:
		static ClassNil INSTANCE;
		static Object NIL;
		std::string name() override;
	};
	
	class ClassBool : public Class {
	public:
		static ClassBool INSTANCE;
		static Object TRUE, FALSE;
		static inline Object* create(bool b) {
			return b ? &TRUE : &FALSE;
		}
		static inline bool value(Execution& exe, Object* ob) {
			ob = ob->adapt(exe, &INSTANCE);
			if (ob == &TRUE) {
				return true;
			} else if (ob == &FALSE) {
				return false;
			} else {
				throw new std::runtime_error("bool wasn't TRUE or FALSE");
			}
		}
		
		std::string name() override;
	};
	
	class ClassNumber : public Class {
	public:
		static ClassNumber INSTANCE;
		static inline Object* create(double n) {
			return new Object(&INSTANCE, n);
		}
		static inline double& value(Execution& exe, Object* ob) {
			return ob->adapt(exe, &INSTANCE)->value.asDouble;
		}
		
		std::string name() override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
	};
	
	class ClassString : public Class {
	public:
		static ClassString INSTANCE;
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
	
	class ClassSystemFunction : public Class {
	public:
		using Handler = std::function<Object*(Execution&, Scope&, Object*, Vector<Object*>&, Map<std::string,Object*>&)>;
		static inline Object* create(Handler handler) {
			return new Object(&INSTANCE, gcAlloc<Handler>(handler));
		}
		
		static ClassSystemFunction INSTANCE;
		std::string name() override;
		bool executable(Object* self, Execution& exe) override;
		Object* execute(Object* self, Execution& exe, Scope& scope, Object* input, Vector<Object*>& args, Map<std::string,Object*>& flags) override;
	};
	
	class ClassManagedFunction : public ClassSystemFunction {
	public:
		using Handler = std::function<Object*(Execution&, Scope&, Object*, Map<std::string,Object*>&, Vector<Object*>&, Map<std::string,Object*>&)>;
		
		class Instance : public gc {
		public:
			inline Instance(const Function& fn, Handler handler) : handler(handler), fn(fn) {}
			
			inline Instance(const Vector<Function::Arg>& args, const Vector<Function::Arg>& flags, Handler handler) :
				handler(handler), fn(args, flags)
			{}
			
			inline Instance(std::initializer_list<Function::Arg> args, std::initializer_list<Function::Arg> flags, Handler handler) :
				handler(handler), fn(args, flags)
			{}
			
			Handler handler;
			Function fn;
		};
		
		static ClassManagedFunction INSTANCE;
		static inline Object* create(Instance* i) {
			return new Object(&INSTANCE, i);
		}
		static inline Object* create(std::initializer_list<Function::Arg> args, std::initializer_list<Function::Arg> flags, Handler handler) {
			return new Object(&INSTANCE, new Instance(args, flags, handler));
		}
		static inline Object* create(const Vector<Function::Arg>& args, const Vector<Function::Arg>& flags, Handler handler) {
			return new Object(&INSTANCE, new Instance(args, flags, handler));
		}
		static inline Instance& value(const Object* ob) {
			return *(Instance*)ob->value.asPtr;
		}
		static inline Instance& value(Execution& exe, Object* ob) {
			return *(Instance*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		Object* execute(Object* self, Execution& exe, Scope& scope, Object* input, Vector<Object*>& args, Map<std::string,Object*>& flags) override;
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
		template<typename... Args> static Object* create(Args... args) {
			return create(gcAlloc<Deque<Object*>>(args...));
		}
		static inline Deque<Object*>& value(const Object* ob) {
			return *(Deque<Object*>*)ob->value.asPtr;
		}
		static inline Deque<Object*>& value(Execution& exe, Object* ob) {
			return *(Deque<Object*>*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
		
		Vector<Object*> fieldNames(Object* self, Execution& exe) override;
		bool hasField(Object* self, Execution& exe, Object* name) override;
		Object* getField(Object* self, Execution& exe, Object* name) override;
		bool canSetField(Object* self, Execution& exe, Object* name) override;
		void setField(Object* self, Execution& exe, Object* name, Object* value) override;
	};
	
	class ClassError : public Class {
	public:
		static ClassError INSTANCE;
		std::string name() override;
	};
	
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
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
	};
	
	class ClassMap : public Class {
	public:
		static ClassMap INSTANCE;
		static inline Object* create(Map<Object*,Object*>* args) {
			return new Object(&INSTANCE, args);
		}
		template<typename... Args> static Object* create(Args... args) {
			return create(gcAlloc<Map<Object*,Object*>>(args...));
		}
		static inline Map<Object*,Object*>& value(const Object* ob) {
			return *(Map<Object*,Object*>*)ob->value.asPtr;
		}
		static inline Map<Object*,Object*>& value(Execution& exe, Object* ob) {
			return *(Map<Object*,Object*>*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
		
		Vector<Object*> fieldNames(Object* self, Execution& exe) override;
		Object* getField(Object* self, Execution& exe, Object* name) override;
		bool canSetField(Object* self, Execution& exe, Object* name) override;
		void setField(Object* self, Execution& exe, Object* name, Object* value) override;
	};
	
	class ClassTable : public Class {
	public:
		class Instance {
		public:
			Vector<Object*> colNames;
			Vector<Vector<Object*>> rows;
			
			inline Instance() {}
			inline Instance(std::initializer_list<std::string> colStrings) : colNames(colStrings.size()) {
				std::transform(colStrings.begin(), colStrings.end(), colNames.begin(), [](const std::string& s) {
					return ClassString::create(s);
				});
			}
		};
		
		static ClassTable INSTANCE;
		static inline Object* create(Instance* args) {
			return new Object(&INSTANCE, args);
		}
		template<typename... Args> static Object* create(Args... args) {
			return create(gcAlloc<Instance>(args...));
		}
		static inline Instance& value(const Object* ob) {
			return *(Instance*)ob->value.asPtr;
		}
		static inline Instance& value(Execution& exe, Object* ob) {
			return *(Instance*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		std::string name() override;
		std::size_t hash(const Object* self) const override;
		bool equals(const Object* self, const Object* other) const override;
		
		Vector<Object*> fieldNames(Object* self, Execution& exe) override;
		Object* getField(Object* self, Execution& exe, Object* name) override;
		bool canSetField(Object* self, Execution& exe, Object* name) override;
		void setField(Object* self, Execution& exe, Object* name, Object* value) override;
	};
	
	class ClassMethod : public ClassSystemFunction {
	public:
		class Instance {
		public:
			class Handler {
			public:
				Class* clazz;
				Object* selector;
				Object* handler;
				
				inline Handler(Class* clazz, Object* selector, Object* handler) : clazz(clazz), selector(selector), handler(handler) {}
			};
			
			bool mutableByUser = false;
			Deque<Handler> handlers;
			Object* defaultHandler;
			
			inline Instance() : defaultHandler(nullptr) {}
			inline Instance(Object* defaultHandler) : defaultHandler(defaultHandler) {}
		};
		
		static inline Object* create(Instance* args) {
			return new Object(&INSTANCE, args);
		}
		template<typename... Args> static Object* create(Args... args) {
			return create(gcAlloc<Instance>(args...));
		}
		static inline Instance& value(const Object* ob) {
			return *(Instance*)ob->value.asPtr;
		}
		static inline Instance& value(Execution& exe, Object* ob) {
			return *(Instance*)ob->adapt(exe, &INSTANCE)->value.asPtr;
		}
		
		static ClassMethod INSTANCE;
		Object* execute(Object* self, Execution& exe, Scope& scope, Object* input, Vector<Object*>& args, Map<std::string,Object*>& flags) override;
	};
}

#endif /* SRC_CLASSES_HPP_ */
