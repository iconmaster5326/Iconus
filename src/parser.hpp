/*
 * parser.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_PARSER_HPP_
#define SRC_PARSER_HPP_

#include "lexer.hpp"
#include "session.hpp"
#include "op.hpp"

namespace iconus {
	Op* parse(Execution& exe, Lexer& input);
}

#endif /* SRC_PARSER_HPP_ */
