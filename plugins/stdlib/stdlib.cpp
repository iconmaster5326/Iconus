/*
 * plugin.cpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#include "plugin.hpp"

#include <string>
#include <iostream>

using namespace std;
using namespace iconus;

extern "C" string iconus_getName() {
	cout << "in iconus_getName!" << endl;
	return "Standard Library";
}

extern "C" void iconus_initGlobalScope(GlobalScope& scope) {
	cout << "in iconus_initGlobalScope!" << endl;
}

extern "C" void iconus_initSession(Session& session) {
	cout << "in iconus_initSession!" << endl;
}
