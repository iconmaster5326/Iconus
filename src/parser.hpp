/*
 * parser.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_PARSER_HPP_
#define SRC_PARSER_HPP_

#include "lexer.hpp"
#include "program.hpp"

namespace iconus {
	Op* parse(Scope& scope, Lexer& input);
}

#endif /* SRC_PARSER_HPP_ */
