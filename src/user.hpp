/*
 * user.hpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#ifndef SRC_USER_HPP_
#define SRC_USER_HPP_

#include <string>
#include <functional>
#include <mutex>
#include <sys/types.h>

namespace iconus {
	class User {
	public:
		static std::mutex MUTEX;
		static uid_t REAL_UID;
		static gid_t REAL_GID;
		static std::string GUEST_USERNAME;
		static bool IS_ROOT;
		
		// TODO: use encrypted challenge-response method instead of sending password over the wire
		// TODO: (we should do this even when we switch to HTTPS and WSS)
		// TODO: Does PAM even support challenge-response instead of asking for passwords via prompt?
		User(const std::string& name, const std::string& password);
		inline User() : User(GUEST_USERNAME, "") {}
		
		void doAsUser(std::function<void()> f);
		
		std::string name;
		std::string home;
		std::string cwd;
		uid_t uid;
		gid_t gid;
	};
}

#endif /* SRC_USER_HPP_ */
