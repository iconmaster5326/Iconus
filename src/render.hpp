/*
 * render.hpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#ifndef SRC_RENDER_HPP_
#define SRC_RENDER_HPP_

#include <functional>
#include <deque>

#include "program.hpp"

namespace iconus {
	class Renderer {
	public:
		using Filter = std::function<bool(Session&, Object*)>;
		using Handler = std::function<std::string(Session&, Object*)>;
		
		inline Renderer(const std::string& name, Filter filter, Handler handler) : name(name), filter(filter), handler(handler) {}
		
		std::string name;
		Filter filter;
		Handler handler;
	};
}

#endif /* SRC_RENDER_HPP_ */
