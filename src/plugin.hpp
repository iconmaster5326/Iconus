/*
 * plugin.hpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#ifndef SRC_PLUGIN_HPP_
#define SRC_PLUGIN_HPP_

#include <string>

#include "session.hpp"

namespace iconus {
	class Plugin {
	public:
		static Vector<Plugin> plugins;
		static void loadPlugin(const std::string& filename);
		template<typename T> static void loadPlugins(const T& filenames) {
			for (std::string filename : filenames) {
				loadPlugin(filename);
			}
		}
		
		std::string name; // iconus_getName
		void initGlobalScope(GlobalScope& scope); // iconus_initGlobalScope
		void initSession(Session& session); // iconus_initSession
	private:
		Plugin(const std::string& filename);
		void* handle;
	};
}

#endif /* SRC_PLUGIN_HPP_ */
