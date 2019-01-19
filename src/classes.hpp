/*
 * classes.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_CLASSES_HPP_
#define SRC_CLASSES_HPP_

#include "program.hpp"

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
		using Handler = std::function<Object*(Session&, Scope&, Object*, const std::unordered_map<std::string,Object*>&, const std::vector<Object*>& args, const std::unordered_map<std::string,Object*>& flags)>;
		
		class Instance {
		public:
			class Arg {
			public:
				std::string name;
				Object* defaultValue; // null if required argument
				
				inline Arg(std::string name) : name(name), defaultValue(nullptr) {}
				inline Arg(std::string name, Object* defaultValue) : name(name), defaultValue(defaultValue) {}
			};
			
			Handler handler;
			std::string input, vararg, varflag;
			std::vector<Arg> args;
			std::vector<Arg> flags;
			
			inline Instance(std::string input, std::string vararg, std::string varflag, const std::vector<Arg>& args, const std::vector<Arg>& flags, Handler handler) :
					input(input), args(args), flags(flags), handler(handler)
			{}
			
			inline Instance(std::string input, std::string vararg, std::string varflag, std::initializer_list<Arg> args, std::initializer_list<Arg> flags, Handler handler) :
					input(input), args(args), flags(flags), handler(handler)
			{}
		};
		
		static ClassManagedFunction INSTANCE;
		Object* execute(Object* self, Session& session, Scope& scope, Object* input, const std::vector<Object*>& args, const std::unordered_map<std::string,Object*>& flags) override;
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
