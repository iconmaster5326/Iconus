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
		};
		
		Type type;
		std::string value;
	};
	
	class Lexer {
	public:
		inline Lexer(std::istream& input) : inputOwned(false), input(&input) {}
		inline Lexer(std::string input) : inputOwned(true), input(new std::istringstream(input)) {}
		~Lexer();
		
		Token next();
		bool done();
	private:
		bool inputOwned;
		std::istream* input;
	};
}

#endif /* SRC_LEXER_HPP_ */
