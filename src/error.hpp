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
	
	class Source {
	public:
		static Source UNKNOWN;
		
		const std::string* location;
		int line, col;
		
		inline Source() : location{nullptr}, line{-1}, col{-1} {}
		inline Source(const std::string& location, int line = -1, int col = -1) : location{&location}, line{line}, col{col} {}
	};
	
	class StackTrace {
	public:
		enum class Type {
			SYNTAX,
			FUNCTION
		};
		
		Type type;
		std::string name;
		Source source;
		
		inline StackTrace(Type type, const Source& source, const std::string& name = "") : type(type), name(name), source(source) {}
		
		static thread_local Vector<StackTrace> callStack;
		static void enter(Type type, const Source& source = Source::UNKNOWN, const std::string& name = "");
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
