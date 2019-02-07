/*
 * error.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_ERROR_HPP_
#define SRC_ERROR_HPP_

#include "gc.hpp"

#include <stdexcept>
#include <string>

namespace iconus {
	class Object; class Execution;
	
	class StackTrace {
	public:
		enum class Type {
			FILE,
			FUNCTION,
			INPUT
		};
		
		Type type;
		std::string where;
		int line;
		
		inline StackTrace(Type type, const std::string& where, int line = -1) : type(type), where(where), line(line) {}
		
		static thread_local Vector<StackTrace> callStack;
		static void enter(Type type, const std::string& where, int line = -1);
		static void exit();
	};
	
	class Error : public std::exception {
	public:
		Error(Execution& exe, Object* value);
		Error(const std::string& value);
		Error(Execution& exe, Object* value, Vector<StackTrace>& stackTrace);
		Error(const std::string& value, Vector<StackTrace>& stackTrace);
		
		Object* value;
		Vector<StackTrace> stackTrace;
		const char* what() const noexcept override;
	private:
		std::string whatString;
	};
}

#endif /* SRC_ERROR_HPP_ */
