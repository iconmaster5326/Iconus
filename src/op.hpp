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
#include "error.hpp"

#include <string>

namespace iconus {
	class Function;
	
	class Op {
	public:
		virtual Object* evaluate(Execution& exe, Scope& scope, Object* input) = 0;
		virtual operator std::string() = 0;
		
		inline Op() {}
		inline Op(const Source& source) : source{source} {}
		
		Source source;
	};
	
	class OpConst : public Op {
	public:
		inline OpConst(Object* value, const Source& source = Source::UNKNOWN) : Op(source), value(value) {}
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
		
		inline OpCall(const std::string& cmd, const Source& source = Source::UNKNOWN) : Op(source), cmd(cmd), args() {}
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
		
		inline OpBinary(Op* lhs, Type type, Op* rhs, const Source& source = Source::UNKNOWN) : Op(source), lhs(lhs), type(type), rhs(rhs) {}
		Object* evaluate(Execution& exe, Scope& scope, Object* input) override;
		operator std::string() override;
		
		Type type;
		Op* lhs;
		Op* rhs;
	};
	
	class OpVar : public Op {
	public:
		inline OpVar(std::string name, const Source& source = Source::UNKNOWN) : Op(source), name(name) {}
		Object* evaluate(Execution& exe, Scope& scope, Object* input) override;
		operator std::string() override;
		
		std::string name;
	};
	
	class OpLambda : public Op {
	public:
		inline OpLambda(const Source& source = Source::UNKNOWN) : Op(source), code(nullptr) {}
		inline OpLambda(Op* code, const Source& source = Source::UNKNOWN) : Op(source), code(code) {}
		Object* evaluate(Execution& exe, Scope& scope, Object* input) override;
		operator std::string() override;
		
		Function fn;
		Op* code;
	};
	
	class OpExString : public Op {
	public:
		inline OpExString(const std::string& str, const Source& source = Source::UNKNOWN) : Op(source), str{str} {}
		Object* evaluate(Execution& exe, Scope& scope, Object* input) override;
		operator std::string() override;
		
		std::string str;
	};
}


#endif /* SRC_OP_HPP_ */
