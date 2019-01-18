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
		using Handler = std::function<std::string(Object*)>;
		
		static void add(std::string name, Class* clazz, Handler handler);
		static std::deque<const Renderer*> getAll(Object* ob);
		static const Renderer* get(Object* ob);
		static void addDefaultRenderers();
		
		inline static std::string render(Object* ob) {
			return Renderer::get(ob)->handler(ob);
		}
		
		inline Renderer(const std::string& name, Class* clazz, Handler handler) : name(name), clazz(clazz), handler(handler) {}
		
		std::string name;
		Class* clazz;
		Handler handler;
	};
}

#endif /* SRC_RENDER_HPP_ */
