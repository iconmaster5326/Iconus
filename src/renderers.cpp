/*
 * default_renderers.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "render.hpp"
#include "classes.hpp"
#include "session.hpp"

#include <sstream>

using namespace std;
using namespace iconus;

void Session::addDefaultRenderers() {
	renderers.emplace_back("numbered list", [](Session& session, Object* ob) {
		return ob->clazz == &ClassList::INSTANCE;
	}, [this](Session& session, Object* ob) {
		ostringstream sb;
		sb << "<ol>";
		
		deque<Object*>& items = *((deque<Object*>*)ob->value.asPtr);
		for (Object* item : items) {
			sb << "<li>" << render(item) << "</li>";
		}
		
		sb << "</ol>";
		return sb.str();
	});
	
	renderers.emplace_back("raw string", [](Session& session, Object* ob) {
		return true;
	}, [](Session& session, Object* ob) {
		return ob->operator string();
	});
}
