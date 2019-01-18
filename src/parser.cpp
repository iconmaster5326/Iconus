/*
 * parser.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "parser.hpp"
#include "classes.hpp"
#include "error.hpp"

#include <list>

using namespace std;
using namespace iconus;

namespace iconus {
	// TODO: an algorithm that does this without consuming all the input first
	// That was the point of having Lexer act like a stream! So this didn't have to happen!!

	static Op* parse(list<Token>& tokens);

	static Op* parsePostConst(Op* op, list<Token>& tokens) {
		if (tokens.empty()) return op;
		switch (tokens.front().type) {
		case Token::Type::PIPE: {
			tokens.pop_front();
			return new OpBinary(op,OpBinary::Type::PIPE,parse(tokens));
		} break;
		default: throw Error("Token invalid after constant: "+tokens.front().value);
		}
	}

	static Op* parse(list<Token>& tokens) {
		if (tokens.empty()) {
			return new OpBinary(nullptr,OpBinary::Type::PIPE,nullptr);
		}
		
		switch (tokens.front().type) {
		case Token::Type::WORD: {
			OpCall* call = new OpCall(tokens.front().value);
			tokens.pop_front();
			
			while (true) {
				if (tokens.empty()) return call;
				
				switch (tokens.front().type) {
				case Token::Type::WORD: {
					call->args.emplace_back(new OpConst(new Object(&ClassString::INSTANCE, new string(tokens.front().value))));
					tokens.pop_front();
				} break;
				case Token::Type::PIPE: {
					tokens.pop_front();
					return new OpBinary(call,OpBinary::Type::PIPE,parse(tokens));
				} break;
				case Token::Type::LPAREN: {
					tokens.pop_front();
					list<Token> subTokens;
					int parenLevel = 0;
					
					while (parenLevel >= 0) {
						if (tokens.empty()) throw Error("expected ')'; not found");
						if (tokens.front().type == Token::Type::LPAREN) {
							parenLevel++;
						} else if (tokens.front().type == Token::Type::RPAREN) {
							parenLevel--;
						}
						
						subTokens.push_back(tokens.front());
						tokens.pop_front();
					}
					
					subTokens.pop_back();
					call->args.emplace_back(parse(subTokens));
				} break;
				case Token::Type::STRING: {
					call->args.emplace_back(new OpConst(new Object(&ClassString::INSTANCE, new string(tokens.front().value))));
					tokens.pop_front();
				} break;
				default: throw Error("Token invalid in function call: "+tokens.front().value);
				}
			}
		} break;
		case Token::Type::PIPE: {
			tokens.pop_front();
			return new OpBinary(nullptr,OpBinary::Type::PIPE,parse(tokens));
		} break;
		case Token::Type::LPAREN: {
			tokens.pop_front();
			list<Token> subTokens;
			int parenLevel = 0;
			
			while (parenLevel >= 0) {
				if (tokens.empty()) throw Error("expected ')'; not found");
				if (tokens.front().type == Token::Type::LPAREN) {
					parenLevel++;
				} else if (tokens.front().type == Token::Type::RPAREN) {
					parenLevel--;
				}
				
				subTokens.push_back(tokens.front());
				tokens.pop_front();
			}
			
			subTokens.pop_back();
			return parsePostConst(parse(subTokens), tokens);
		} break;
		case Token::Type::STRING: {
			Op* op = new OpConst(new Object(&ClassString::INSTANCE, new string(tokens.front().value)));
			tokens.pop_front();
			return parsePostConst(op, tokens);
		} break;
		default: throw Error("Token invalid: "+tokens.front().value);
		}
	}
	
	Op* parse(Lexer& input) {
		list<Token> tokens;
		for (Token t = input.next(); t.type != Token::Type::NONE; t = input.next()) {
			tokens.push_back(t);
		}
		
		return parse(tokens);
	}
}
