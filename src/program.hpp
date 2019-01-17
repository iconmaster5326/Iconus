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
	class Object;
	
	class Class {
	public:
		virtual ~Class();
		virtual std::string name();
		virtual std::string toString(Object* self);
	};
	
	class Object {
	public:
		inline Object(Class* clazz, void* value) : clazz(clazz), value{.asPtr = value} {}
		
		operator std::string();
		
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
		
		virtual ~OpBinary();
		Object* evaluate(Scope& scope, Object* input) override;
		operator std::string() override;
		
		Type type;
		Op* lhs;
		Op* rhs;
	};
}

#endif /* SRC_PROGRAM_HPP_ */
