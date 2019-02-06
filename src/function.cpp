/*
 * function.cpp
 *
 *  Created on: Feb 5, 2019
 *      Author: iconmaster
 */

#include "function.hpp"
#include "op.hpp"

using namespace std;
using namespace iconus;

iconus::Function::Arg::Arg(std::string name, Object* defaultValue) : name(name), defaultValue(new OpConst(defaultValue)), role(Role::NONE) {}
iconus::Function::Arg::Arg(std::string name, Object* defaultValue, Role role) : name(name), defaultValue(new OpConst(defaultValue)), role(role) {}
