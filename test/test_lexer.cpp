/*
 * test_lexer.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: iconmaster
 */

#include "lexer.hpp"

using namespace std;
using namespace iconus;

static void printResult(string input) {
	Lexer lexer(input);
	int n = 0;
	
	while (!lexer.done()) {
		Token t = lexer.next();
		cout << "\t" << ((int)t.type) << ": " << t.value << endl;
		n++;
	}
	
	cout << n << " tokens found." << endl;
}

int main() {
	printResult("two words");
	printResult("w    hite		spa\n 	\n ce");
	printResult("ab;cd|ef&ge");
	printResult("word -flag -");
	printResult("word $var $");
	printResult("a 'big long string' here");
	printResult("'a\\'b'");
	printResult("a \"big long '$extended' string\" here");
	printResult("\"a\\\"b\"");
}
