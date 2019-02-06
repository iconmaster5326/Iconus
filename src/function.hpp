/*
 * function.hpp
 *
 *  Created on: Jan 18, 2019
 *      Author: iconmaster
 */

#ifndef SRC_FUNCTION_HPP_
#define SRC_FUNCTION_HPP_

#include "program.hpp"

#include <string>
#include <initializer_list>

namespace iconus {
	class Op;
	
	class Function {
	public:
		enum class Role {
			NONE,
			INPUT,
			VARARG,
			VARFLAG,
		};
		
		class Arg {
		public:
			std::string name;
			Op* defaultValue; // null if required argument
			Role role;
			
			inline Arg(std::string name) : name(name), defaultValue(nullptr), role(Role::NONE) {}
			inline Arg(std::string name, Op* defaultValue) : name(name), defaultValue(defaultValue), role(Role::NONE) {}
			inline Arg(std::string name, Role role) : name(name), defaultValue(nullptr), role(role) {}
			inline Arg(std::string name, Op* defaultValue, Role role) : name(name), defaultValue(defaultValue), role(role) {}
			
			Arg(std::string name, Object* defaultValue);
			Arg(std::string name, Object* defaultValue, Role role);
		};
		
		Vector<Arg> args;
		Vector<Arg> flags;
		
		inline Function() {}
		
		inline Function(const Vector<Arg>& args, const Vector<Arg>& flags) :
				args(args), flags(flags)
		{}
		
		inline Function(std::initializer_list<Arg> args, std::initializer_list<Arg> flags) :
				args(args), flags(flags)
		{}
	};
}

#endif /* SRC_FUNCTION_HPP_ */
