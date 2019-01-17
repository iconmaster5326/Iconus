/*
 * classes.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_CLASSES_HPP_
#define SRC_CLASSES_HPP_

#include "program.hpp"

namespace iconus {
	class ClassString : public Class {
	public:
		static ClassString INSTANCE;
		std::string name() override;
		std::string toString(Object* self) override;
	};
}

#endif /* SRC_CLASSES_HPP_ */
