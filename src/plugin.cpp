/*
 * plugin.cpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#include "plugin.hpp"
#include "error.hpp"

#include <dlfcn.h>
#include <boost/filesystem.hpp>

using namespace std;
using namespace iconus;
using namespace boost::filesystem;

Vector<Plugin> iconus::Plugin::plugins{};

iconus::Plugin::Plugin(const std::string& filename) : handle(dlopen(filename.c_str(), RTLD_LAZY | RTLD_LOCAL)) {
	if (!handle) {
		string error(dlerror());
		throw Error("Unable to load plugin "+error);
	}
	
	auto nameFn = (string(*)()) dlsym(handle, "iconus_getName");
	if (!nameFn) {
		string error(dlerror());
		throw Error("Unable to call iconus_getName in plugin "+error);
	}
	name = nameFn();
	
	auto htmlFn = (string(*)()) dlsym(handle, "iconus_initHTML");
	if (htmlFn) { // initHTML is optional
		initHTML = htmlFn();
	}
}

static bool endsWith(const string& fullString, const string& ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

void iconus::Plugin::loadPlugin(const std::string& filename) {
	path p{filename};
	if (is_directory(status(p))) {
		for (directory_iterator it{p}; it != directory_iterator{}; it++) {
			if (is_directory(it->status()) || endsWith(it->path().string(), ".icolib")) {
				loadPlugin(it->path().string());
			}
		}
	} else {
		plugins.push_back(Plugin(filename));
	}
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
	auto fn = (void(*)(Execution&)) dlsym(handle, "iconus_initSession");
	if (!fn) {
		string error(dlerror());
		throw Error("Unable to call iconus_initSession in plugin "+name+": "+error);
	}
	
	Execution exe{session};
	fn(exe);
}
