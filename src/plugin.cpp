/*
 * plugin.cpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#include "plugin.hpp"
#include "error.hpp"

#include <dlfcn.h>

using namespace std;
using namespace iconus;

Vector<Plugin> iconus::Plugin::plugins{};

iconus::Plugin::Plugin(const std::string& filename) : handle(dlopen(filename.c_str(), RTLD_LAZY | RTLD_LOCAL)) {
	if (!handle) {
		string error(dlerror());
		throw Error("Unable to load plugin "+error);
	}
	
	auto fn = (string(*)()) dlsym(handle, "iconus_getName");
	if (!fn) {
		string error(dlerror());
		throw Error("Unable to call iconus_getName in plugin "+error);
	}
	name = fn();
}

void iconus::Plugin::loadPlugin(const std::string& filename) {
	// TODO: directories
	plugins.push_back(Plugin(filename));
}

void iconus::Plugin::initGlobalScope(GlobalScope& scope) {
	auto fn = (void(*)(GlobalScope&)) dlsym(handle, "iconus_initGlobalScope");
	if (!fn) {
		string error(dlerror());
		throw Error("Unable to call iconus_initGlobalScope in plugin "+name+": "+error);
	}
	
	fn(scope);
}

void iconus::Plugin::initSession(Session& session) {
	auto fn = (void(*)(Session&)) dlsym(handle, "iconus_initSession");
	if (!fn) {
		string error(dlerror());
		throw Error("Unable to call iconus_initSession in plugin "+name+": "+error);
	}
	
	fn(session);
}
