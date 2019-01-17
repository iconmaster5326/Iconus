/*
 * server.hpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#ifndef SRC_SERVER_HPP_
#define SRC_SERVER_HPP_

#include <string>

namespace iconus {
	void startServer(const std::string& addr, unsigned short port, const std::string& html);
}

#endif /* SRC_SERVER_HPP_ */
