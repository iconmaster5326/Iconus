/*
 * adaptors.cpp
 *
 *  Created on: Jan 20, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "classes.hpp"

using namespace std;
using namespace iconus;

void iconus::Session::addDefaultAdaptors() {
	adaptors[&ClassNumber::INSTANCE] = {};
	adaptors[&ClassString::INSTANCE] = {};
	adaptors[&ClassBool::INSTANCE] = {};
	
	adaptors[&ClassNumber::INSTANCE][&ClassString::INSTANCE] = [](Session& session, Object* from) {
		double value = ClassNumber::value(session, from);
		return ClassString::create(to_string(value));
	};
	
	adaptors[&ClassNumber::INSTANCE][&ClassBool::INSTANCE] = [](Session& session, Object* from) {
		double value = ClassNumber::value(session, from);
		if (value == 0.0) return &ClassBool::FALSE; else return &ClassBool::TRUE;
	};
	
	adaptors[&ClassString::INSTANCE][&ClassNumber::INSTANCE] = [](Session& session, Object* from) {
		const string& value = ClassString::value(session, from);
		return ClassNumber::create(stod(value));
	};
}
