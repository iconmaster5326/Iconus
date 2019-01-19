/*
 * word_parsers.cpp
 *
 *  Created on: Jan 18, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "classes.hpp"
#include "error.hpp"

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
	
	parsers.emplace_back([](Session& session, const std::string& word) {
		try {
			stod(word);
			return true;
		} catch (const invalid_argument& ex) {
			return false;
		} catch (const out_of_range& ex) {
			throw Error("Numeric constant out of range: "+word);
		}
	},[](Session& session, string word) {
		return new Object(&ClassNumber::INSTANCE, stod(word));
	});
}
