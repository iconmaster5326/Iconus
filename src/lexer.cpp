/*
 * lexer.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "lexer.hpp"
#include "error.hpp"

#include <cctype>

namespace iconus {
	using namespace std;
	
	static const string SPECIAL_CHARS = "$|&;(){}[]\"\'";
	
	iconus::Lexer::~Lexer() {
		if (inputOwned) delete input;
	}
	
	void Lexer::advance(char c) {
		if (c == '\n') {
			line++;
			col = 1;
		} else {
			col++;
		}
	}

	Token iconus::Lexer::next() {
		noskipws(*input);
		
		char c = ' ';
		while (!done() && isspace(c)) {
			*input >> c;
			advance(c);
		}
		if (done()) return Token{Token::Type::NONE, "", {location, line, col}};
		
		switch (c) {
		case '|': return Token{Token::Type::PIPE, "|", {location, line, col}};
		case '&': return Token{Token::Type::AND, "&", {location, line, col}};
		case ';': return Token{Token::Type::SEMICOLON, ";", {location, line, col}};
		case '(': return Token{Token::Type::LPAREN, "(", {location, line, col}};
		case ')': return Token{Token::Type::RPAREN, ")", {location, line, col}};
		case '{': return Token{Token::Type::LBRACE, "{", {location, line, col}};
		case '}': return Token{Token::Type::RBRACE, "}", {location, line, col}};
		case '[': return Token{Token::Type::LBRACKET, "[", {location, line, col}};
		case ']': return Token{Token::Type::RBRACKET, "]", {location, line, col}};
		case '-': {
			ostringstream word;
			char c = input->peek();
			
			while (!done() && !isspace(c) && SPECIAL_CHARS.find(c) == string::npos) {
				word << c;
				
				*input >> c; advance(c);
				c = input->peek();
			}
			
			string result = word.str();
			if (result.empty())
				return Token{Token::Type::WORD, "-", {location, line, col}};
			else {
				try {
					stod(result);
					return Token{Token::Type::WORD, "-"+result, {location, line, col}}; // really bad hack for negative number constants
				} catch (const invalid_argument& ex) {
					return Token{Token::Type::FLAG, result, {location, line, col}};
				} catch (const out_of_range& ex) {
					throw Error("Numeric constant out of range: "+result);
				}
			}
		} break;
		case '$': {
			ostringstream word;
			char c = input->peek();
			
			while (!done() && !isspace(c) && SPECIAL_CHARS.find(c) == string::npos) {
				word << c;
				
				*input >> c; advance(c);
				c = input->peek();
			}
			
			string result = word.str();
			if (result.empty())
				return Token{Token::Type::WORD, "$", {location, line, col}};
			else
				return Token{Token::Type::VAR, result, {location, line, col}};
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
				
				*input >> c; advance(c);
				c = input->peek();
			}
			
			*input >> c; advance(c);
			return Token{Token::Type::STRING, word.str(), {location, line, col}};
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
				
				*input >> c; advance(c);
				c = input->peek();
			}
			
			*input >> c; advance(c);
			return Token{Token::Type::EX_STRING, word.str(), {location, line, col}};
		}
		default:
			ostringstream word; word << c;
			char c = input->peek();
			
			while (!done() && !isspace(c) && SPECIAL_CHARS.find(c) == string::npos) {
				word << c;
				
				*input >> c; advance(c);
				c = input->peek();
			}
			
			string s = word.str();
			if (s == "...")
				return Token{Token::Type::ELLIPSES, s, {location, line, col}};
			else
				return Token{Token::Type::WORD, s, {location, line, col}};
		}
	}

	bool iconus::Lexer::done() {
		return input->eof();
	}
}
