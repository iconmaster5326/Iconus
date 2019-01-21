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
		
		Deque<Object*>& items = *((Deque<Object*>*)ob->value.asPtr);
		for (Object* item : items) {
			sb << "<li>" << render(item) << "</li>";
		}
		
		sb << "</ol>";
		return sb.str();
	});
	
	renderers.emplace_back("error", [](Session& session, Object* ob) {
		return ob->clazz == &ClassError::INSTANCE;
	}, [this](Session& session, Object* ob) {
		ostringstream sb;
		sb << "<div style=\"color: red;\"><b>error:</b> ";
		
		Object* what = (Object*) ob->value.asPtr;
		sb << session.render(what);
		
		sb << "</div>";
		return sb.str();
	});
	
	renderers.emplace_back("raw string", [](Session& session, Object* ob) {
		return true;
	}, [](Session& session, Object* ob) {
		return ob->toString(session);
	});
}
