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
uid_t iconus::User::REAL_UID = geteuid();
gid_t iconus::User::REAL_GID = getegid();

extern "C" {
	#include <stdlib.h>
	#include <stdbool.h>
	#include <string.h>
	#include <security/pam_appl.h>
	
	static int pamconv(int num_msg, const struct pam_message **msg,
			struct pam_response **resp, void *appdata_ptr)
	{
		char *pass = (char*) malloc(strlen((char*)appdata_ptr)+1);
		strcpy(pass, (char*)appdata_ptr);
	
		int i;
	
		*resp = (struct pam_response*) calloc(num_msg, sizeof(struct pam_response));
	
		for (i = 0; i < num_msg; ++i)
		{
			/* Ignore all PAM messages except prompting for hidden input */
			if (msg[i]->msg_style != PAM_PROMPT_ECHO_OFF)
				continue;
	
			/* Assume PAM is only prompting for the password as hidden input */
			resp[i]->resp = pass;
		}
	
		return PAM_SUCCESS;
	}
	
	static bool checkAuthentication(const char *user, const char *pass)
	{
		/* use own PAM conversation function just responding with the
		   password passed here */
		struct pam_conv conv = { &pamconv, (void *)pass };
	
		pam_handle_t *handle;
		int authResult;
	
		pam_start("shutterd", user, &conv, &handle);
		authResult = pam_authenticate(handle,
				PAM_SILENT|PAM_DISALLOW_NULL_AUTHTOK);
		pam_end(handle, authResult);
	
		return (authResult == PAM_SUCCESS);
	}
}

iconus::User::User(const std::string& name, const std::string& password) : name(name) {
	if (!checkAuthentication(name.c_str(), password.c_str())) {
		throw Error("Cannot login as "+name+": User doesn't exist or password is incorrect");
	}
	
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
			throw Error("Cannot login as "+name+": User doesn't exist or password is incorrect");
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
