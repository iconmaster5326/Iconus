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
		// TODO: actually verify users using passwords!!!
		
		static std::mutex MUTEX;
		static uid_t REAL_UID;
		static gid_t REAL_GID;
		
		inline User() : uid(REAL_UID), gid(REAL_GID), home("."), cwd("."), name("guest") {} // TODO: guest user
		User(const std::string& name);
		User(uid_t uid);
		
		void doAsUser(std::function<void()> f);
		
		std::string name;
		std::string home;
		std::string cwd;
		uid_t uid;
		gid_t gid;
	};
}

#endif /* SRC_USER_HPP_ */
