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

using namespace std;
using namespace iconus;

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
	
	////////////////////////////
	// adaptors
	////////////////////////////
	
	////////////////////////////
	// cat handlers
	////////////////////////////
	
}
