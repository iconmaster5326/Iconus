/*
 * gui.cpp
 *
 *  Created on: Feb 13, 2019
 *      Author: iconmaster
 */

#include "lib_classes.hpp"

#include "session.hpp"
#include "error.hpp"
#include "util.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
using namespace iconus;
using namespace boost::uuids;

#include "build/gui/header.cxx"
static string header((const char*)header_html, header_html_len);

extern "C" string iconus_getName() {
	return "GUI";
}

extern "C" string iconus_initHTML() {
	return header;
}

using Arg = Function::Arg;
constexpr Function::Role INPUT = Function::Role::INPUT;
constexpr Function::Role VARARG = Function::Role::VARARG;
constexpr Function::Role VARFLAG = Function::Role::VARFLAG;

extern "C" void iconus_initGlobalScope(GlobalScope& scope) {
	////////////////////////////
	// functions
	////////////////////////////
	scope.vars["vbox"] = ClassManagedFunction::create(
			{Arg("items", VARARG)}, {Arg("style", &ClassNil::NIL)},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
				return ClassVBox::create(args["items"], args["style"]->toString(exe));
			}
	);
	
	scope.vars["hbox"] = ClassManagedFunction::create(
			{Arg("items", VARARG)}, {Arg("style", &ClassNil::NIL)},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
				return ClassHBox::create(args["items"], args["style"]->toString(exe));
			}
	);
	
	scope.vars["button"] = ClassManagedFunction::create(
			{Arg("text"), Arg("on-click", &ClassNil::NIL)}, {},
			[](Execution& exe, Scope& scope, Object* input, auto& args, auto& varargs, auto& varflags) {
				Object* onClick = ClassEvent::create(ClassList::create());
				if (args["on-click"]->truthy()) {
					ClassEvent::connect(onClick, args["on-click"]);
				}
				return ClassButton::create(args["text"], onClick);
			}
	);
	
	////////////////////////////
	// constants
	////////////////////////////
	
}

extern "C" void iconus_initSession(Execution& exe) {
	////////////////////////////
	// word parsers
	////////////////////////////
	
	////////////////////////////
	// renderers
	////////////////////////////
	exe.session.renderers.emplace_back("gui vbox", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassVBox::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		Lock lock{ob->mutex};
		auto& box = ClassVBox::value(exe, ob);
		
		ostringstream sb;
		sb << "<table class=\"gui-table\" style=\"" << box.style << "\"><tr class=\"gui-table\" style=\"" << box.style << "\">";
		
		for (Object* item : box.items->fieldValues(exe)) {
			sb << "<td class=\"gui-table\" style=\"" << box.style << "\">" << exe.render(item) << "</td>";
		}
		
		sb << "</tr></table>";
		return sb.str();
	});
	
	exe.session.renderers.emplace_back("gui hbox", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassHBox::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		Lock lock{ob->mutex};
		auto& box = ClassHBox::value(exe, ob);
		
		ostringstream sb;
		sb << "<table class=\"gui-table\" style=\"" << box.style << "\">";
		
		for (Object* item : box.items->fieldValues(exe)) {
			sb << "<tr class=\"gui-table\" style=\"" << box.style << "\"><td class=\"gui-table\" style=\"" << box.style << "\">" << exe.render(item) << "</td></tr>";
		}
		
		sb << "</tr></table>";
		return sb.str();
	});
	
	exe.session.renderers.emplace_back("gui button", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassButton::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		Lock lock{ob->mutex};
		auto& button = ClassButton::value(exe, ob);
		
		Object* event = button.onClick;
		uuid& id = button.id;
		
		exe.idsRendered.insert(&button.id);
		thread t{[&exe,&id,event]() {
			Vector<Object*> args; Map<string,Object*> flags;
			
			while (true) {
				Map<string,string> map;
				exe.getMessage(id, map);
				while (map.empty()) this_thread::yield();
				
				try {
					ClassEvent::fire(event, exe, exe.session.sessionScope, &ClassNil::NIL, args, flags);
				} catch (const Error& e) {
					// do nothing. TODO: log it?
				}
			}
		}};
		t.detach();
		
		ostringstream sb;
		sb << "<button type=\"button\" onclick=\"onGuiButtonClick('" << to_string(button.id) << "')\">";
		sb << exe.render(button.text);
		sb << "</input>";
		return sb.str();
	});
	
	////////////////////////////
	// adaptors
	////////////////////////////
	
	////////////////////////////
	// cat handlers
	////////////////////////////
	
}
