/*
 * lexer.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_LEXER_HPP_
#define SRC_LEXER_HPP_

#include <string>
#include <iostream>
#include <sstream>

#include "error.hpp"

namespace iconus {
	class Token {
	public:
		enum class Type {
			NONE,
			WORD,
			STRING, // '...'
			EX_STRING, // "..."
			VAR, // $...
			FLAG, // -...
			PIPE, // |
			SEMICOLON, // ;
			AND, // &
			LPAREN, // ()
			RPAREN,
			LBRACE, // {}
			RBRACE,
			LBRACKET, // []
			RBRACKET,
			ELLIPSES, // a literal set of three dots
		};
		
		Type type;
		std::string value;
		Source source;
	};
	
	class Lexer {
	public:
		inline Lexer(const std::string& location, std::istream& input) : location{location}, inputOwned(false), input(&input) {}
		inline Lexer(const std::string& location, const std::string& input) : location{location}, inputOwned(true), input(new std::istringstream(input)) {}
		~Lexer();
		
		Token next();
		bool done();
	private:
		const std::string& location;
		bool inputOwned;
		std::istream* input;
		int line = 1;
		int col = 1;
		
		void advance(char c);
	};
}

#endif /* SRC_LEXER_HPP_ */
