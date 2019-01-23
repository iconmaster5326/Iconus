/*
 * base64.cpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

#include "base64.hpp"

using namespace std;
using namespace boost::archive::iterators;
using namespace boost::algorithm;

namespace iconus {
//	std::string base64decode(const std::string& val) {
//		using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
//		return trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
//			return c == '\0';
//		});
//	}
	
	std::string base64encode(const Base64Buffer& buf) {
		using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
		auto tmp = std::string(It(buf.b), It(buf.b+buf.n));
		return tmp.append((3 - buf.n % 3) % 3, '=');
	}
}
