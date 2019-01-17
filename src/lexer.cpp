/*
 * lexer.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "lexer.hpp"

#include <cctype>

namespace iconus {
	using namespace std;
	
	static const string SPECIAL_CHARS = "$|&;(){}[]\"\'";
	
	iconus::Lexer::~Lexer() {
		if (inputOwned) delete input;
	}

	Token iconus::Lexer::next() {
		noskipws(*input);
		
		char c = ' ';
		while (!done() && isspace(c)) *input >> c;
		if (done()) return Token{Token::Type::NONE};
		
		switch (c) {
		case '|': return Token{Token::Type::PIPE};
		case '&': return Token{Token::Type::AND};
		case ';': return Token{Token::Type::SEMICOLON};
		case '(': return Token{Token::Type::LPAREN};
		case ')': return Token{Token::Type::RPAREN};
		case '{': return Token{Token::Type::LBRACE};
		case '}': return Token{Token::Type::RBRACE};
		case '[': return Token{Token::Type::LBRACKET};
		case ']': return Token{Token::Type::RBRACKET};
		case '-': {
			ostringstream word;
			char c = input->peek();
			
			while (!done() && !isspace(c) && SPECIAL_CHARS.find(c) == string::npos) {
				word << c;
				
				*input >> c;
				c = input->peek();
			}
			
			string result = word.str();
			if (result.empty())
				return Token{Token::Type::WORD, "-"};
			else
				return Token{Token::Type::FLAG, result};
		}
		case '$': {
			ostringstream word;
			char c = input->peek();
			
			while (!done() && !isspace(c) && SPECIAL_CHARS.find(c) == string::npos) {
				word << c;
				
				*input >> c;
				c = input->peek();
			}
			
			string result = word.str();
			if (result.empty())
				return Token{Token::Type::WORD, "$"};
			else
				return Token{Token::Type::VAR, result};
		}
		case '\'': {
			ostringstream word;
			char c = input->peek();
			bool escape = false;
			
			while (!done() && (escape || c != '\'')) {
				if (escape) {
					escape = false;
					switch (c) {
					case 'n': word << '\n'; break;
					case 't': word << '\t'; break;
					case 'b': word << '\b'; break;
					case 'r': word << '\r'; break;
					default: word << c;
					}
				} else if (c == '\\') {
					escape = true;
				} else {
					word << c;
				}
				
				*input >> c;
				c = input->peek();
			}
			
			*input >> c;
			return Token{Token::Type::STRING, word.str()};
		}
		case '\"': {
			ostringstream word;
			char c = input->peek();
			bool escape = false;
			
			while (!done() && (escape || c != '\"')) {
				if (escape) {
					escape = false;
					switch (c) {
					case 'n': word << '\n'; break;
					case 't': word << '\t'; break;
					case 'b': word << '\b'; break;
					case 'r': word << '\r'; break;
					default: word << c;
					}
				} else if (c == '\\') {
					escape = true;
				} else {
					word << c;
				}
				
				*input >> c;
				c = input->peek();
			}
			
			*input >> c;
			return Token{Token::Type::EX_STRING, word.str()};
		}
		default:
			ostringstream word; word << c;
			char c = input->peek();
			
			while (!done() && !isspace(c) && SPECIAL_CHARS.find(c) == string::npos) {
				word << c;
				
				*input >> c;
				c = input->peek();
			}
			return Token{Token::Type::WORD, word.str()};
		}
	}

	bool iconus::Lexer::done() {
		return input->eof();
	}
}
