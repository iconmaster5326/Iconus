/*
 * error.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_ERROR_HPP_
#define SRC_ERROR_HPP_

#include <stdexcept>

namespace iconus {
	class Object;
	
	class Error : public std::exception {
	public:
		Error(Object* value);
		Error(const std::string& value);
		
		Object* value;
		const char* what() const noexcept override;
	private:
		std::string whatString;
	};
}

#endif /* SRC_ERROR_HPP_ */
