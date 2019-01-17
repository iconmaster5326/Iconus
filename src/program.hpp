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
	class Object; class Scope;
	
	class Class {
	public:
		virtual ~Class();
		virtual std::string name();
		virtual std::string toString(Object* self);
		virtual bool executable();
		virtual Object* execute(Object* self, Scope& scope, Object* input, const std::vector<Object*>& args, const std::unordered_map<std::string,Object*>& flags);
	};
	
	class Object {
	public:
		inline Object(Class* clazz, void* value) : clazz(clazz), value{.asPtr = value} {}
		
		inline operator std::string() {
			return clazz->toString(this);
		}
		inline bool executable() {
			return clazz->executable();
		}
		inline Object* execute(Scope& scope, Object* input, const std::vector<Object*>& args, const std::unordered_map<std::string,Object*>& flags) {
			return clazz->execute(this, scope, input, args, flags);
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
	};
	
	class Op {
	public:
		virtual ~Op();
		virtual Object* evaluate(Scope& scope, Object* input) = 0;
		virtual operator std::string() = 0;
	};
	
	class OpConst : public Op {
	public:
		inline OpConst(Object* value) : value(value) {}
		virtual ~OpConst();
		Object* evaluate(Scope& scope, Object* input) override;
		operator std::string() override;
		
		Object* value;
	};
	
	class OpCall : public Op {
	public:
		class Arg {
		public:
			inline Arg(Op* value) : isFlag(false), key(""), value(value) {}
			inline Arg(std::string key, Op* value) : isFlag(true), key(key), value(value) {}
			
			bool isFlag;
			std::string key;
			Op* value;
		};
		
		inline OpCall(const std::string& cmd) : cmd(cmd), args() {}
		virtual ~OpCall();
		Object* evaluate(Scope& scope, Object* input) override;
		operator std::string() override;
		
		std::string cmd;
		std::vector<Arg> args;
	};
	
	class OpBinary : public Op {
	public:
		enum class Type {
			PIPE,
			FOREACH,
			RESET,
		};
		
		inline OpBinary(Op* lhs, Type type, Op* rhs) : lhs(lhs), type(type), rhs(rhs) {}
		virtual ~OpBinary();
		Object* evaluate(Scope& scope, Object* input) override;
		operator std::string() override;
		
		Type type;
		Op* lhs;
		Op* rhs;
	};
}

#endif /* SRC_PROGRAM_HPP_ */
