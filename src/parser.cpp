/*
 * parser.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "parser.hpp"
#include "classes.hpp"

using namespace std;
using namespace iconus;

namespace iconus {
	static Op* parse(Scope& scope, Lexer& input, Token t) {
		switch (t.type) {
		case Token::Type::NONE: return nullptr;
		case Token::Type::WORD: {
			OpCall* result = new OpCall();
			result->cmd = t.value;
			
			while (true) {
				t = input.next();
				switch (t.type) {
				case Token::Type::NONE: return result;
				case Token::Type::WORD: {
					result->args.emplace_back(new OpConst(new Object(&ClassString::INSTANCE, new string(t.value))));
				} break;
				}
			}
		} break;
		}
		
		throw exception();
	}
	
	Op* parse(Scope& scope, Lexer& input) {
		return parse(scope, input, input.next());
	}
}
