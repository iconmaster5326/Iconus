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
#include <vector>
#include <initializer_list>

namespace iconus {
	class ClassNil : public Class {
	public:
		static ClassNil INSTANCE;
		static Object NIL;
		std::string name() override;
		std::string toString(Object* self) override;
	};
	
	class ClassBool : public Class {
	public:
		static ClassBool INSTANCE;
		static Object TRUE, FALSE;
		std::string name() override;
		std::string toString(Object* self) override;
	};
	
	class ClassNumber : public Class {
	public:
		static ClassNumber INSTANCE;
		std::string name() override;
		std::string toString(Object* self) override;
	};
	
	class ClassString : public Class {
	public:
		static ClassString INSTANCE;
		std::string name() override;
		std::string toString(Object* self) override;
	};
	
	class ClassSystemFunction : public Class {
	public:
		using Handler = std::function<Object*(Session&, Scope&, Object*, const std::vector<Object*>&, const std::unordered_map<std::string,Object*>&)>;
		
		static ClassSystemFunction INSTANCE;
		std::string name() override;
		bool executable() override;
		Object* execute(Object* self, Session& session, Scope& scope, Object* input, const std::vector<Object*>& args, const std::unordered_map<std::string,Object*>& flags) override;
	};
	
	class ClassManagedFunction : public ClassSystemFunction {
	public:
		using Handler = std::function<Object*(Session&, Scope&, Object*, const std::unordered_map<std::string,Object*>&, const std::vector<Object*>&, const std::unordered_map<std::string,Object*>&)>;
		
		class Instance {
		public:
			inline Instance(const Function& fn, Handler handler) : handler(handler), fn(fn) {}
			
			inline Instance(std::string input, std::string vararg, std::string varflag, const std::vector<Function::Arg>& args, const std::vector<Function::Arg>& flags, Handler handler) :
				handler(handler), fn(input, vararg, varflag, args, flags)
			{}
			
			inline Instance(std::string input, std::string vararg, std::string varflag, std::initializer_list<Function::Arg> args, std::initializer_list<Function::Arg> flags, Handler handler) :
				handler(handler), fn(input, vararg, varflag, args, flags)
			{}
			
			Handler handler;
			Function fn;
		};
		
		static ClassManagedFunction INSTANCE;
		Object* execute(Object* self, Session& session, Scope& scope, Object* input, const std::vector<Object*>& args, const std::unordered_map<std::string,Object*>& flags) override;
	};
	
	class ClassUserFunction : public ClassManagedFunction {
	public:
		static ClassUserFunction INSTANCE;
		static Object* create(Scope& scope, Op* op, const Function& fn);
	};
	
	class ClassList : public Class {
	public:
		static ClassList INSTANCE;
		std::string name() override;
		std::string toString(Object* self) override;
	};
	
	class ClassError : public Class {
	public:
		static ClassError INSTANCE;
		std::string name() override;
		std::string toString(Object* self) override;
	};
}

#endif /* SRC_CLASSES_HPP_ */
