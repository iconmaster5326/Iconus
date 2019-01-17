/*
 * session.hpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#ifndef SRC_SESSION_HPP_
#define SRC_SESSION_HPP_

#include <string>

namespace iconus {
	class Session {
	public:
		std::string evaluate(const std::string& input);
	private:
		char dummy;
	};
}

#endif /* SRC_SESSION_HPP_ */
