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
		Lexer lexer("a b c d e");
		cout << parse(lexer)->operator string() << endl;
	}
	
	{
		Lexer lexer("a | b");
		cout << parse(lexer)->operator string() << endl;
	}
	
	{
		Lexer lexer("a 1 2 3 | b 4 5 6 | c | d");
		cout << parse(lexer)->operator string() << endl;
	}
	
	{
		Lexer lexer("1 2 (3) (4 5 6) (7 (8 9) 10)");
		cout << parse(lexer)->operator string() << endl;
	}
	
	{
		Lexer lexer("(a | b) | (c | d)");
		cout << parse(lexer)->operator string() << endl;
	}
	
	{
		Lexer lexer("(a | b | (c | d)) | e");
		cout << parse(lexer)->operator string() << endl;
	}
	
	return 0;
}
