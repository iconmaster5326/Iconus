/*
 * render.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "render.hpp"

#include <unordered_map>

using namespace std;
using namespace iconus;

static Renderer defaultRenderer("raw string", nullptr, [](Object* ob){
	return ob->operator string();
});

static unordered_map<Class*, deque<Renderer>> renderers;

void iconus::Renderer::add(string name, Class* clazz, Handler handler) {
	renderers[clazz].emplace_back(name, clazz, handler);
}

std::deque<const Renderer*> iconus::Renderer::getAll(Object* ob) {
	deque<const Renderer*> result;
	for (const Renderer& renderer : renderers[ob->clazz]) {
		result.push_back(&renderer);
	}
	result.push_back(&defaultRenderer);
	return result;
}

const Renderer* iconus::Renderer::get(Object* ob) {
	return getAll(ob).front();
}
