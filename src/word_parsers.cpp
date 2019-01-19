/*
 * word_parsers.cpp
 *
 *  Created on: Jan 18, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "classes.hpp"

using namespace std;
using namespace iconus;

void iconus::Session::addDefaultWordParsers() {
	parsers.emplace_back([](Session& session, const std::string& word) {
		return word == "nil";
	},[](Session& session, string word) {
		return &ClassNil::NIL;
	});
	
	parsers.emplace_back([](Session& session, const std::string& word) {
		return word == "true";
	},[](Session& session, string word) {
		return &ClassBool::TRUE;
	});
	
	parsers.emplace_back([](Session& session, const std::string& word) {
		return word == "false";
	},[](Session& session, string word) {
		return &ClassBool::FALSE;
	});
}
