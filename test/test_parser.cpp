/*
 * test_parser.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "parser.hpp"

using namespace std;
using namespace iconus;

int main() {
	Scope scope;
	
	{
		Lexer lexer("a b c d");
		cout << parse(scope, lexer)->operator string() << endl;
	}
	
	return 0;
}
