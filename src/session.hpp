/*
 * session.hpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#ifndef SRC_SESSION_HPP_
#define SRC_SESSION_HPP_

#include "program.hpp"

#include <string>

namespace iconus {
	class Scope;
	
	class Session {
	public:
		Session();
		
		std::string evaluate(const std::string& input);
		
		Scope globalScope;
	};
}

#endif /* SRC_SESSION_HPP_ */
