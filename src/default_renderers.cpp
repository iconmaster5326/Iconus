/*
 * default_renderers.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "render.hpp"
#include "classes.hpp"

#include <sstream>

using namespace std;
using namespace iconus;

void Renderer::addDefaultRenderers() {
	Renderer::add("numbered list", &ClassList::INSTANCE, [](Object* ob){
		ostringstream sb;
		sb << "<ol>";
		
		deque<Object*>& items = *((deque<Object*>*)ob->value.asPtr);
		for (Object* item : items) {
			sb << "<li>" << Renderer::render(item) << "</li>";
		}
		
		sb << "</ol>";
		return sb.str();
	});
}
