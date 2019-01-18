/*
 * main.cpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#include <string>
#include "server.hpp"
#include "render.hpp"

using namespace std;
using namespace iconus;

#include "build/index.cxx"
static string html((const char*)src_index_html, src_index_html_len);

int main(int argc, const char** argv) {
	Renderer::addDefaultRenderers();
	startServer("", 8080, html);
	return 0;
}
