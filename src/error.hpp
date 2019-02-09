/*
 * error.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_ERROR_HPP_
#define SRC_ERROR_HPP_

#include "types.hpp"

#include <stdexcept>
#include <string>

namespace iconus {
	class Object; class Execution;
	
	class StackTrace {
	public:
		std::string name;
		std::string file;
		int line;
		
		inline StackTrace(const std::string& name = "", const std::string& file = "", int line = -1) : name(name), file(file), line(line) {}
		
		static thread_local Vector<StackTrace> callStack;
		static void enter(const std::string& name = "", const std::string& file = "", int line = -1);
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
