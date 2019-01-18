/*
 * classes.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_CLASSES_HPP_
#define SRC_CLASSES_HPP_

#include "program.hpp"

#include <functional>

namespace iconus {
	class ClassNil : public Class {
	public:
		static ClassNil INSTANCE;
		static Object NIL;
		std::string name() override;
		std::string toString(Object* self) override;
	};
	
	class ClassString : public Class {
	public:
		static ClassString INSTANCE;
		std::string name() override;
		std::string toString(Object* self) override;
	};
	
	class ClassSystemFunction : public Class {
	public:
		using Handler = std::function<Object*(Scope&, Object*, const std::vector<Object*>&, const std::unordered_map<std::string,Object*>&)>;
		
		static ClassSystemFunction INSTANCE;
		std::string name() override;
		bool executable() override;
		Object* execute(Object* self, Scope& scope, Object* input, const std::vector<Object*>& args, const std::unordered_map<std::string,Object*>& flags) override;
	};
}

#endif /* SRC_CLASSES_HPP_ */
