/*
 * user.cpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#include "user.hpp"
#include "error.hpp"

#include <unistd.h>
#include <pwd.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace iconus;
using namespace boost::filesystem;

std::mutex iconus::User::MUTEX{};
uid_t iconus::User::REAL_UID = getuid();
gid_t iconus::User::REAL_GID = getgid();

iconus::User::User(const std::string& name) : name(name) {
	struct passwd entry;
	struct passwd* entryP;
	size_t buflen = 1024;
	char* buffer = new char[buflen];
	
	while (true) {
		int status = getpwnam_r(name.c_str(), &entry, buffer, buflen, &entryP);
		if (status == ERANGE) {
			buflen *= 2;
			delete[] buffer;
			buffer = new char[buflen];
		} else if (status == 0) {
			if (entryP) {
				home = string(entry.pw_dir); cwd = home;
				uid = entry.pw_uid;
				gid = entry.pw_gid;
				break;
			} else {
				throw Error("User '"+name+"' not found");
			}
		} else {
			throw Error("Error in getpwnam_r for lookup of user '"+name+"': "+string(strerror(status)));
		}
	}
}

iconus::User::User(uid_t uid) : uid(uid) {
	struct passwd entry;
	struct passwd* entryP;
	size_t buflen = 1024;
	char* buffer = new char[buflen];
	
	while (true) {
		int status = getpwuid_r(uid, &entry, buffer, buflen, &entryP);
		if (status == ERANGE) {
			buflen *= 2;
			delete[] buffer;
			buffer = new char[buflen];
		} else if (status == 0) {
			if (entryP) {
				home = string(entry.pw_dir); cwd = home;
				name = string(entry.pw_name);
				gid = entry.pw_gid;
				break;
			} else {
				throw Error("User "+to_string(uid)+" not found");
			}
		} else {
			throw Error("Error in getpwuid_r for lookup of user "+to_string(uid)+": "+string(strerror(status)));
		}
	}
}

void iconus::User::doAsUser(std::function<void()> f) {
	// lock mutex so no other thread can mess up uids and gids
	lock_guard<mutex> lock{MUTEX};
	int status;
	
	// assume effective credentials of user
	status = seteuid(uid);
	if (status < 0) {
		throw Error("Couldn't set uid");
	}
	
	status = setegid(gid);
	if (status < 0) {
		throw Error("Couldn't set gid");
	}
	
	path oldPath{current_path()};
	current_path(path{cwd});
	
	// run f
	f();
	
	// re-assume credentials of server
	status = seteuid(REAL_UID);
	if (status < 0) {
		// should always be fatal if we can't get back into root status
		cerr << "FATAL ERROR: Couldn't reset uid" << endl;
		exit(1);
	}
	
	status = setegid(REAL_GID);
	if (status < 0) {
		// should always be fatal if we can't get back into root status
		cerr << "FATAL ERROR: Couldn't reset gid" << endl;
		exit(1);
	}
	
	current_path(oldPath);
	
	// the lock will expire here thanks to RAII, allowing for other users to do user operations
}
