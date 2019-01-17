/*
 * base64.hpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#ifndef SRC_BASE64_HPP_
#define SRC_BASE64_HPP_

#include <string>

namespace iconus {
	std::string base64encode(const std::string& val);
	std::string base64decode(const std::string& val);
}

#endif /* SRC_BASE64_HPP_ */
