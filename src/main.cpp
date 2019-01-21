/*
 * main.cpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#include <string>
#include <iostream>

#include "server.hpp"
#include "render.hpp"
#include "plugin.hpp"
#include "error.hpp"
#include "session.hpp"

using namespace std;
using namespace iconus;

#include "build/index.cxx"
static string html((const char*)src_index_html, src_index_html_len);

int main(int argc, const char** argv) {
	for (int i = 0; i < argc; i++) {
		string arg(argv[i]);
		if (arg == "-p") {
			i++;
			if (argv[i]) {
				try {
					string arg(argv[i]);
					Plugin::loadPlugin(arg);
				} catch (const exception& e) {
					cout << "WARNING: error in plugin load: " << e.what() << endl;
				}
			}
		}
	}
	
	for (Plugin& p : Plugin::plugins) {
		try {
			p.initGlobalScope(GlobalScope::INSTANCE);
		} catch (const exception& e) {
			cout << "WARNING: error in plugin initGlobalScope: " << e.what() << endl;
		}
	}
	
	startServer("", 8080, html);
	return 0;
}
