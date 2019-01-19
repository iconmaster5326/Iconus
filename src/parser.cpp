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

	static Op* parse(Session& session, list<Token>& tokens);

	static Op* parsePostConst(Session& session, Op* op, list<Token>& tokens) {
		if (tokens.empty()) return op;
		switch (tokens.front().type) {
		case Token::Type::PIPE: {
			tokens.pop_front();
			return new OpBinary(op,OpBinary::Type::PIPE,parse(session, tokens));
		} break;
		default: throw Error("Token invalid after constant: "+tokens.front().value);
		}
	}
	
	static Op* parseArg(Session& session, list<Token>& tokens) {
		string value = tokens.front().value;
		
		switch (tokens.front().type) {
		case Token::Type::WORD: {
			tokens.pop_front();
			return new OpConst(new Object(&ClassString::INSTANCE, new string(value)));
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
			return parse(session, subTokens);
		} break;
		case Token::Type::STRING: {
			tokens.pop_front();
			return new OpConst(new Object(&ClassString::INSTANCE, new string(value)));
		} break;
		}
		
		return nullptr;
	}

	static Op* parse(Session& session, list<Token>& tokens) {
		if (tokens.empty()) {
			return new OpBinary(nullptr,OpBinary::Type::PIPE,nullptr);
		}
		
		switch (tokens.front().type) {
		case Token::Type::WORD: {
			OpCall* call = new OpCall(tokens.front().value);
			tokens.pop_front();
			
			while (true) {
				if (tokens.empty()) return call;
				
				Op* arg = parseArg(session, tokens);
				if (arg) {
					call->args.emplace_back(arg);
				} else {
					switch (tokens.front().type) {
					case Token::Type::PIPE: {
						tokens.pop_front();
						return new OpBinary(call,OpBinary::Type::PIPE,parse(session, tokens));
					} break;
					case Token::Type::FLAG: {
						string value = tokens.front().value;
						tokens.pop_front();
						
						if (tokens.empty()) {
							call->args.emplace_back(value, new OpConst(&ClassNil::NIL)); // TODO: true should be the default when value not specified
							return call;
						}
						
						Op* arg = parseArg(session, tokens);
						if (arg) {
							call->args.emplace_back(value, arg);
						} else {
							call->args.emplace_back(value, new OpConst(&ClassNil::NIL)); // TODO: true should be the default when value not specified
							
							switch (tokens.front().type) {
							case Token::Type::PIPE: {
								tokens.pop_front();
								return new OpBinary(call,OpBinary::Type::PIPE,parse(session, tokens));
							} break;
							case Token::Type::FLAG: {
								// do nothing; let next loop handle this flag
							} break;
							default: throw Error("Token invalid after flag: "+tokens.front().value);
							}
						}
					} break;
					default: throw Error("Token invalid in function call: "+tokens.front().value);
					}
				}
			}
		} break;
		case Token::Type::PIPE: {
			tokens.pop_front();
			return new OpBinary(nullptr,OpBinary::Type::PIPE,parse(session, tokens));
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
			return parsePostConst(session, parse(session, subTokens), tokens);
		} break;
		case Token::Type::STRING: {
			Op* op = new OpConst(new Object(&ClassString::INSTANCE, new string(tokens.front().value)));
			tokens.pop_front();
			return parsePostConst(session, op, tokens);
		} break;
		default: throw Error("Token invalid: "+tokens.front().value);
		}
	}
	
	Op* parse(Session& session, Lexer& input) {
		list<Token> tokens;
		for (Token t = input.next(); t.type != Token::Type::NONE; t = input.next()) {
			tokens.push_back(t);
		}
		
		return parse(session, tokens);
	}
}
