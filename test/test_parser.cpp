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
	{
		Lexer lexer("a 1 2 3 | b 4 5 6 | c | d");
		cout << parse(lexer)->operator string() << endl;
	}
	
	return 0;
}
