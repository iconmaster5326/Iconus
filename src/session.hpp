/*
 * session.hpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#ifndef SRC_SESSION_HPP_
#define SRC_SESSION_HPP_

#include "program.hpp"
#include "render.hpp"

#include <string>
#include <vector>

namespace iconus {
	class Scope;
	
	class Session {
	public:
		Session();
		
		Object* evaluate(const std::string& input);
		std::string render(Object* object);
		
		Scope globalScope;
		std::vector<Renderer> renderers;
	private:
		void addDefaultRenderers();
	};
}

#endif /* SRC_SESSION_HPP_ */
