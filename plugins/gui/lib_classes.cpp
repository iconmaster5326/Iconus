/*
 * classes.cpp
 *
 *  Created on: Feb 13, 2019
 *      Author: iconmaster
 */


#include "lib_classes.hpp"

#include "error.hpp"
#include "session.hpp"

#include <sstream>

using namespace std;
using namespace iconus;

#define DEF_FIELD(str,name) static string name##_S{str}; static Object name{&ClassString::INSTANCE, &name##_S}

ClassHBox ClassHBox::INSTANCE{};

std::string iconus::ClassHBox::name() {
	return "hbox";
}

ClassVBox ClassVBox::INSTANCE{};

std::string iconus::ClassVBox::name() {
	return "vbox";
}

ClassButton ClassButton::INSTANCE{};

std::string iconus::ClassButton::name() {
	return "button";
}

DEF_FIELD("text",FIELD_BUTTON_TEXT);
DEF_FIELD("on-click",FIELD_BUTTON_ONCLICK);
static Vector<Object*> FIELDS_BUTTON{{&FIELD_BUTTON_TEXT, &FIELD_BUTTON_ONCLICK}};

Vector<Object*> iconus::ClassButton::fieldNames(Object* self, Execution& exe) {
	return FIELDS_BUTTON;
}

Object* iconus::ClassButton::getField(Object* self, Execution& exe,
		Object* name) {
	Instance& button = ClassButton::value(self);
	
	if (name->equals(&FIELD_BUTTON_TEXT)) {
		return button.text;
	} else if (name->equals(&FIELD_BUTTON_ONCLICK)) {
		return button.onClick;
	} else {
		return Class::getField(self, exe, name);
	}
}

bool iconus::ClassButton::canSetField(Object* self, Execution& exe,
		Object* name) {
	return name->equals(&FIELD_BUTTON_TEXT) || name->equals(&FIELD_BUTTON_ONCLICK);
}

void iconus::ClassButton::setField(Object* self, Execution& exe, Object* name,
		Object* value) {
	Instance& button = ClassButton::value(self);
	
	if (name->equals(&FIELD_BUTTON_TEXT)) {
		 button.text = value;
	} else if (name->equals(&FIELD_BUTTON_ONCLICK)) {
		 button.onClick = value->adapt(exe, &ClassEvent::INSTANCE);
	} else {
		Class::setField(self, exe, name, value);
	}
}
