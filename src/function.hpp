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
		class Arg {
		public:
			std::string name;
			Op* defaultValue; // null if required argument
			
			inline Arg(std::string name) : name(name), defaultValue(nullptr) {}
			inline Arg(std::string name, Op* defaultValue) : name(name), defaultValue(defaultValue) {}
		};
		
		std::string input, vararg, varflag;
		Vector<Arg> args;
		Vector<Arg> flags;
		
		inline Function() {}
		
		inline Function(std::string input, std::string vararg, std::string varflag, const Vector<Arg>& args, const Vector<Arg>& flags) :
				input(input), vararg(vararg), varflag(varflag), args(args), flags(flags)
		{}
		
		inline Function(std::string input, std::string vararg, std::string varflag, std::initializer_list<Arg> args, std::initializer_list<Arg> flags) :
				input(input), vararg(vararg), varflag(varflag), args(args), flags(flags)
		{}
	};
}

#endif /* SRC_FUNCTION_HPP_ */
