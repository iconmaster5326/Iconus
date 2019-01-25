/*
 * op.hpp
 *
 *  Created on: Jan 18, 2019
 *      Author: iconmaster
 */

#ifndef SRC_OP_HPP_
#define SRC_OP_HPP_

#include "program.hpp"
#include "function.hpp"

#include <string>


namespace iconus {
	class Op : public gc {
	public:
		virtual Object* evaluate(Execution& exe, Scope& scope, Object* input) = 0;
		virtual operator std::string() = 0;
	};
	
	class OpConst : public Op {
	public:
		inline OpConst(Object* value) : value(value) {}
		Object* evaluate(Execution& exe, Scope& scope, Object* input) override;
		operator std::string() override;
		
		Object* value;
	};
	
	class OpCall : public Op {
	public:
		class Arg {
		public:
			inline Arg(Op* value) : isFlag(false), key(""), value(value) {}
			inline Arg(const std::string& key, Op* value) : isFlag(true), key(key), value(value) {}
			
			bool isFlag;
			std::string key;
			Op* value;
		};
		
		inline OpCall(const std::string& cmd) : cmd(cmd), args() {}
		Object* evaluate(Execution& exe, Scope& scope, Object* input) override;
		operator std::string() override;
		
		std::string cmd;
		Vector<Arg> args;
	};
	
	class OpBinary : public Op {
	public:
		enum class Type {
			PIPE,
			FOREACH,
			RESET,
		};
		
		inline OpBinary(Op* lhs, Type type, Op* rhs) : lhs(lhs), type(type), rhs(rhs) {}
		Object* evaluate(Execution& exe, Scope& scope, Object* input) override;
		operator std::string() override;
		
		Type type;
		Op* lhs;
		Op* rhs;
	};
	
	class OpVar : public Op {
	public:
		inline OpVar(std::string name) : name(name) {}
		Object* evaluate(Execution& exe, Scope& scope, Object* input) override;
		operator std::string() override;
		
		std::string name;
	};
	
	class OpLambda : public Op {
	public:
		inline OpLambda() : code(nullptr) {}
		inline OpLambda(Op* code) : code(code) {}
		Object* evaluate(Execution& exe, Scope& scope, Object* input) override;
		operator std::string() override;
		
		Function fn;
		Op* code;
	};
}


#endif /* SRC_OP_HPP_ */
