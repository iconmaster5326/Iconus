/*
 * parser.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "parser.hpp"
#include "classes.hpp"

#include <stdexcept>

using namespace std;
using namespace iconus;

namespace iconus {
	Op* parse(Lexer& input) {
		OpCall* call = nullptr;
		OpBinary* bin = nullptr;
		
		for (Token t = input.next(); t.type != Token::Type::NONE; t = input.next()) {
			switch (t.type) {
			case Token::Type::WORD: {
				if (call) {
					call->args.emplace_back(new OpConst(new Object(&ClassString::INSTANCE, new string(t.value))));
				} else {
					call = new OpCall(t.value);
				}
			} break;
			case Token::Type::PIPE: {
				if (bin) {
					bin->rhs = call;
					bin = new OpBinary(bin, OpBinary::Type::PIPE, nullptr);
					call = nullptr;
				} else {
					bin = new OpBinary(call, OpBinary::Type::PIPE, nullptr);
					call = nullptr;
				}
			} break;
			default: throw runtime_error("parse error: unknown token type");
			}
		}
		
		if (bin) {
			bin->rhs = call;
			return bin;
		} else {
			return call;
		}
	}
}
