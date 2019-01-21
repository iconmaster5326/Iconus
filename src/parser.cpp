/*
 * parser.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "parser.hpp"
#include "classes.hpp"
#include "error.hpp"



using namespace std;
using namespace iconus;

namespace iconus {
	// TODO: an algorithm that does this without consuming all the input first
	// That was the point of having Lexer act like a stream! So this didn't have to happen!!

	static Op* parse(Session& session, List<Token>& tokens);
	static Op* parseArg(Session& session, List<Token>& tokens);
	
	static OpLambda* parseBraces(Session& session, List<Token>& tokens, OpLambda* lambda) {
		if (tokens.empty() || tokens.front().type != Token::Type::LBRACE) throw Error("expected '{'; not found");
		
		tokens.pop_front();
		List<Token> subTokens;
		int parenLevel = 0;
		
		while (parenLevel >= 0) {
			if (tokens.empty()) throw Error("expected '}'; not found");
			if (tokens.front().type == Token::Type::LBRACE) {
				parenLevel++;
			} else if (tokens.front().type == Token::Type::RBRACE) {
				parenLevel--;
			}
			
			subTokens.push_back(tokens.front());
			tokens.pop_front();
		}
		
		subTokens.pop_back();
		lambda->code = parse(session, subTokens);
		return lambda;
	}
	
	static OpLambda* parseBrackets(Session& session, List<Token>& tokens) {
		tokens.pop_front();
		List<Token> subTokens;
		int parenLevel = 0;
		
		while (parenLevel >= 0) {
			if (tokens.empty()) throw Error("expected ']'; not found");
			if (tokens.front().type == Token::Type::LBRACKET) {
				parenLevel++;
			} else if (tokens.front().type == Token::Type::RBRACKET) {
				parenLevel--;
			}
			
			subTokens.push_back(tokens.front());
			tokens.pop_front();
		}
		
		subTokens.pop_back();
		
		List<Token>& restTokens = tokens;
		{
			List<Token>& tokens = subTokens;
			OpLambda* lambda = new OpLambda();
			
			while (!tokens.empty()) {
				switch (tokens.front().type) {
				case Token::Type::WORD: {
					lambda->fn.args.emplace_back(tokens.front().value);
					tokens.pop_front();
				} break;
				case Token::Type::PIPE: {
					tokens.pop_front();
					if (!lambda->fn.args.empty()) {
						if (lambda->fn.args.size() > 1) throw Error("can only have 1 variable before the | in arguments specification");
						lambda->fn.input = lambda->fn.args.front().name;
						lambda->fn.args.pop_back();
					}
				} break;
				case Token::Type::FLAG: {
					lambda->fn.flags.emplace_back(tokens.front().value);
					tokens.pop_front();
				} break;
				case Token::Type::LPAREN: {
					tokens.pop_front();
					switch (tokens.front().type) {
					case Token::Type::WORD: {
						string name = tokens.front().value;
						
						tokens.pop_front();
						if (tokens.empty()) throw Error("expected ')'; not found");
						
						Op* value = parseArg(session, tokens);
						if (tokens.empty() || tokens.front().type != Token::Type::RPAREN) throw Error("expected ')'; found: "+tokens.front().value);
						tokens.pop_front();
						
						lambda->fn.args.emplace_back(name, value);
					} break;
					case Token::Type::FLAG: {
						string name = tokens.front().value;
						
						tokens.pop_front();
						if (tokens.empty()) throw Error("expected ')'; not found");
						
						Op* value = parseArg(session, tokens);
						if (tokens.empty() || tokens.front().type != Token::Type::RPAREN) throw Error("expected ')'; found: "+tokens.front().value);
						tokens.pop_front();
						
						lambda->fn.flags.emplace_back(name, value);
					} break;
					default: throw Error("Token invalid in extended argument specification: "+tokens.front().value);
					}
				} break;
				default: throw Error("Token invalid in arguments specification: "+tokens.front().value);
				}
			}
			
			return parseBraces(session, restTokens, lambda);
		}
	}

	static Op* parsePostConst(Session& session, Op* op, List<Token>& tokens) {
		if (tokens.empty()) return op;
		throw Error("Token invalid after constant: "+tokens.front().value);
	}
	
	static Op* parseArg(Session& session, List<Token>& tokens) {
		string value = tokens.front().value;
		
		switch (tokens.front().type) {
		case Token::Type::WORD: {
			tokens.pop_front();
			
			Object* parsedConst = session.parseWord(value);
			if (parsedConst) {
				return new OpConst(parsedConst);
			} else {
				return new OpConst(new Object(&ClassString::INSTANCE, new string(value)));
			}
		} break;
		case Token::Type::LPAREN: {
			tokens.pop_front();
			List<Token> subTokens;
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
		case Token::Type::VAR: {
			tokens.pop_front();
			return new OpVar(value);
		} break;
		case Token::Type::LBRACE: {
			return parseBraces(session, tokens, new OpLambda());
		} break;
		case Token::Type::LBRACKET: {
			return parseBrackets(session, tokens);
		} break;
		}
		
		return nullptr;
	}

	static Op* parseCall(Session& session, List<Token>& tokens) {
		if (tokens.empty()) {
			return new OpBinary(nullptr,OpBinary::Type::PIPE,nullptr);
		}
		
		switch (tokens.front().type) {
		case Token::Type::WORD: {
			Object* parsedConst = session.parseWord(tokens.front().value);
			if (parsedConst) {
				Op* op = new OpConst(parsedConst);
				tokens.pop_front();
				return parsePostConst(session, op, tokens);
			} else {
				OpCall* call = new OpCall(tokens.front().value);
				tokens.pop_front();
				
				while (true) {
					if (tokens.empty()) return call;
					
					Op* arg = parseArg(session, tokens);
					if (arg) {
						call->args.emplace_back(arg);
					} else {
						switch (tokens.front().type) {
						case Token::Type::FLAG: {
							string value = tokens.front().value;
							tokens.pop_front();
							
							if (tokens.empty()) {
								call->args.emplace_back(value, new OpConst(&ClassBool::TRUE));
								return call;
							}
							
							Op* arg = parseArg(session, tokens);
							if (arg) {
								call->args.emplace_back(value, arg);
							} else {
								call->args.emplace_back(value, new OpConst(&ClassBool::TRUE));
								
								switch (tokens.front().type) {
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
			}
		} break;
		case Token::Type::LPAREN: {
			tokens.pop_front();
			List<Token> subTokens;
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
		case Token::Type::VAR: {
			Op* op = new OpVar(tokens.front().value);
			tokens.pop_front();
			return parsePostConst(session, op, tokens);
		} break;
		case Token::Type::LBRACE: {
			return parsePostConst(session, parseBraces(session, tokens, new OpLambda()), tokens);
		} break;
		case Token::Type::LBRACKET: {
			return parsePostConst(session, parseBrackets(session, tokens), tokens);
		} break;
		default: throw Error("Token invalid: "+tokens.front().value);
		}
	}
	
	Op* parse(Session& session, List<Token>& tokens) {
		class Call {
		public:
			List<Token> tokens;
			OpBinary::Type sepAtEnd = OpBinary::Type::PIPE;
		};
		
		List<Call> calls;
		calls.emplace_back();
		
		for (auto it = tokens.begin(); it != tokens.end(); it++) {
			switch (it->type) {
			case Token::Type::PIPE: {
				calls.back().sepAtEnd = OpBinary::Type::PIPE;
				calls.emplace_back();
			} break;
			case Token::Type::SEMICOLON: {
				calls.back().sepAtEnd = OpBinary::Type::RESET;
				calls.emplace_back();
			} break;
			case Token::Type::AND: {
				calls.back().sepAtEnd = OpBinary::Type::FOREACH;
				calls.emplace_back();
			} break;
			case Token::Type::LPAREN: {
				calls.back().tokens.push_back(*it);
				int parenLevel = 0;
				
				while (parenLevel >= 0) {
					it++;
					
					if (it == tokens.end()) throw Error("expected ')'; not found");
					
					if (it->type == Token::Type::LPAREN) {
						parenLevel++;
					} else if (it->type == Token::Type::RPAREN) {
						parenLevel--;
					}
					
					calls.back().tokens.push_back(*it);
				}
			} break;
			case Token::Type::LBRACE: {
				calls.back().tokens.push_back(*it);
				int parenLevel = 0;
				
				while (parenLevel >= 0) {
					it++;
					
					if (it == tokens.end()) throw Error("expected '}'; not found");
					
					if (it->type == Token::Type::LBRACE) {
						parenLevel++;
					} else if (it->type == Token::Type::RBRACE) {
						parenLevel--;
					}
					
					calls.back().tokens.push_back(*it);
				}
			} break;
			case Token::Type::LBRACKET: {
				calls.back().tokens.push_back(*it);
				int parenLevel = 0;
				
				while (parenLevel >= 0) {
					it++;
					
					if (it == tokens.end()) throw Error("expected ']'; not found");
					
					if (it->type == Token::Type::LBRACKET) {
						parenLevel++;
					} else if (it->type == Token::Type::RBRACKET) {
						parenLevel--;
					}
					
					calls.back().tokens.push_back(*it);
				}
			} break;
			default: calls.back().tokens.push_back(*it);
			}
		}
		
		Op* op = nullptr;
		OpBinary::Type sep = OpBinary::Type::PIPE;
		for (Call& call : calls) {
			Op* rhs = nullptr;
			if (!call.tokens.empty()) {
				rhs = parseCall(session, call.tokens);
			}
			
			if (!op && sep == OpBinary::Type::PIPE) {
				op = rhs;
			} else {
				op = new OpBinary(op, sep, rhs);
			}
			
			sep = call.sepAtEnd;
		}
		
		if (sep != OpBinary::Type::PIPE) {
			return new OpBinary(op, sep, nullptr);
		} else if (op) {
			return op;
		} else {
			return new OpBinary(nullptr, sep, nullptr);
		}
	}
	
	Op* parse(Session& session, Lexer& input) {
		List<Token> tokens;
		for (Token t = input.next(); t.type != Token::Type::NONE; t = input.next()) {
			tokens.push_back(t);
		}
		
		return parse(session, tokens);
	}
}
