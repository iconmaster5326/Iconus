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

ClassHBox ClassHBox::INSTANCE{};

std::string iconus::ClassHBox::name() {
	return "hbox";
}

ClassVBox ClassVBox::INSTANCE{};

std::string iconus::ClassVBox::name() {
	return "vbox";
}
