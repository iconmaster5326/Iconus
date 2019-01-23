/*
 * util.cpp
 *
 *  Created on: Jan 22, 2019
 *      Author: iconmaster
 */

#include "util.hpp"

#include <sstream>

using namespace iconus;
using namespace std;

std::string iconus::escapeHTML(const std::string& s) {
	ostringstream sb;
	for (char c : s) {
		switch (c) {
		case '&': sb << "&amp;"; break;
		case '<': sb << "&lt;"; break;
		case '>': sb << "&gt;"; break;
		case '"': sb << "&quot;"; break;
		case '\'': sb << "&#39;"; break;
		default: sb << c;
		}
	}
	return sb.str();
}
