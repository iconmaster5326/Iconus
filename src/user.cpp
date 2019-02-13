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
#include <grp.h>
#include <sys/capability.h>

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace iconus;
using namespace boost::filesystem;

extern char** environ;

static bool canSetUids() {
	bool canSetUids = false;
	cap_t caps = cap_get_proc();
	if (caps) {
		cap_flag_value_t value;
		if (cap_get_flag(caps, CAP_SETUID, CAP_PERMITTED, &value) == 0) {
			if (value) {
				canSetUids = true;
			}
		}
		
		cap_free(caps);
	}
	
	return canSetUids;
}

std::mutex iconus::User::MUTEX{};
uid_t iconus::User::REAL_UID = getuid();
gid_t iconus::User::REAL_GID = getgid();
std::string iconus::User::GUEST_USERNAME{User::uidToString(getuid())};
bool iconus::User::IS_ROOT = canSetUids();

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
	
		authResult = pam_start("shutterd", user, &conv, &handle);
		if (authResult != PAM_SUCCESS) {
			fprintf(stderr, "FATAL ERROR: PAM failed to initialize\n");
			exit(1);
		}
		authResult = pam_authenticate(handle,
				PAM_SILENT|PAM_DISALLOW_NULL_AUTHTOK);
		pam_end(handle, authResult);
	
		return (authResult == PAM_SUCCESS);
	}
}

iconus::User::User(const std::string& name, const std::string& password) : name(name) {
	if (name != GUEST_USERNAME) {
		if (!checkAuthentication(name.c_str(), password.c_str())) {
			throw Error("Cannot login as "+name+": User doesn't exist or password is incorrect");
		}
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
				throw Error("Cannot login as "+name+": User doesn't exist or password is incorrect");
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
	
	// if we're not root, we can't change to the uids of any other user but ourselves, so just run f
	if (!IS_ROOT) {
		if (uid != REAL_UID || gid != REAL_GID) {
			throw Error("cannot execute operation as user "+name+": Iconus not running as root");
		}
		
		f();
		return;
	}
	
	// assume effective credentials of user
	long ngroups_max = sysconf(_SC_NGROUPS_MAX);
	gid_t oldGroups[ngroups_max];
	int nOldGroups = getgroups(ngroups_max, oldGroups);
	if (nOldGroups < 0) {
		cerr << "FATAL ERROR: Couldn't get supplementary groups: " << strerror(errno) << endl;
		exit(1);
	}
	
	status = initgroups(name.c_str(), gid);
	if (status < 0) {
		cerr << "FATAL ERROR: Couldn't set supplementary groups: " << strerror(errno) << endl;
		exit(1);
	}
	
	status = setresgid(gid, gid, REAL_GID);
	if (status < 0) {
		cerr << "FATAL ERROR: Couldn't set GID: " << strerror(errno) << endl;
		exit(1);
	}
	
	status = setresuid(uid, uid, REAL_UID);
	if (status < 0) {
		cerr << "FATAL ERROR: Couldn't set UID: " << strerror(errno) << endl;
		exit(1);
	}
	
	Map<string,string> oldEnv;
	for (char** envp = environ; *envp; envp++) {
		string pair{*envp};
		size_t i = pair.find('=');
		if (i != string::npos) {
			oldEnv[pair.substr(0, i)] = pair.substr(i+1);
		}
	}
	for (auto& pair : oldEnv) {
		unsetenv(pair.first.c_str());
	}
	for (auto& pair : env) {
		setenv(pair.first.c_str(), pair.second.c_str(), true);
	}
	
	path oldPath{current_path()};
	current_path(path{cwd});
	
	const auto resetCreds = [&]() {
		status = setresuid(REAL_UID, REAL_UID, REAL_UID);
		if (status < 0) {
			cerr << "FATAL ERROR: Couldn't reset uid: " <<  strerror(errno) << endl;
			exit(1);
		}
		
		status = setresgid(REAL_GID, REAL_GID, REAL_GID);
		if (status < 0) {
			cerr << "FATAL ERROR: Couldn't reset gid: " << strerror(errno) << endl;
			exit(1);
		}
		
		status = setgroups(nOldGroups, oldGroups);
		if (status < 0) {
			cerr << "FATAL ERROR: Couldn't reset supplementary groups: " << strerror(errno) << endl;
			exit(1);
		}
		
		current_path(oldPath);
		
		Map<string,string> newEnv;
		for (char** envp = environ; *envp; envp++) {
			string pair{*envp};
			size_t i = pair.find('=');
			if (i != string::npos) {
				newEnv[pair.substr(0, i)] = pair.substr(i+1);
			}
		}
		for (auto& pair : newEnv) {
			unsetenv(pair.first.c_str());
		}
		for (auto& pair : oldEnv) {
			setenv(pair.first.c_str(), pair.second.c_str(), true);
		}
	};
	
	// run f
	try {
		f();
	} catch (...) {
		resetCreds();
		throw;
	}
	
	// re-assume credentials of server
	resetCreds();
	
	// mutex expires here, thanks to RAII
}

std::string iconus::User::uidToString(uid_t id) {
	struct passwd entry;
	struct passwd* entryP;
	size_t buflen = 1024;
	char* buffer = new char[buflen];
	
	while (true) {
		int status = getpwuid_r(id, &entry, buffer, buflen, &entryP);
		if (status == ERANGE) {
			buflen *= 2;
			delete[] buffer;
			buffer = new char[buflen];
		} else if (status == 0) {
			if (entryP) {
				return string(entry.pw_name);
			} else {
				return "";
			}
		} else {
			return "";
		}
	}
}

std::string iconus::User::gidToString(gid_t id) {
	struct group entry;
	struct group* entryP;
	size_t buflen = 1024;
	char* buffer = new char[buflen];
	
	while (true) {
		int status = getgrgid_r(id, &entry, buffer, buflen, &entryP);
		if (status == ERANGE) {
			buflen *= 2;
			delete[] buffer;
			buffer = new char[buflen];
		} else if (status == 0) {
			if (entryP) {
				return string(entry.gr_name);
			} else {
				return "";
			}
		} else {
			return "";
		}
	}
}

uid_t iconus::User::stringToUid(std::string name) {
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
				return entry.pw_uid;
			} else {
				return -1;
			}
		} else {
			return -1;
		}
	}
}

gid_t iconus::User::stringToGid(std::string name) {
	struct group entry;
	struct group* entryP;
	size_t buflen = 1024;
	char* buffer = new char[buflen];
	
	while (true) {
		int status = getgrnam_r(name.c_str(), &entry, buffer, buflen, &entryP);
		if (status == ERANGE) {
			buflen *= 2;
			delete[] buffer;
			buffer = new char[buflen];
		} else if (status == 0) {
			if (entryP) {
				return entry.gr_gid;
			} else {
				return -1;
			}
		} else {
			return -1;
		}
	}
}
