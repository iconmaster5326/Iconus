/*
 * plugin.hpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#ifndef SRC_PLUGIN_HPP_
#define SRC_PLUGIN_HPP_

#if defined _WIN32 || defined __CYGWIN__
  #ifdef ICONUS_PLUGIN
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport)
    #endif
  #endif
  #define DLL_LOCAL
#else
  #define DLL_PUBLIC __attribute__ ((visibility ("default")))
  #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#endif

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
		std::string initHTML;  // iconus_initHTML
		void initGlobalScope(GlobalScope& scope); // iconus_initGlobalScope
		void initSession(Session& session); // iconus_initSession
	private:
		Plugin(const std::string& filename);
		void* handle;
	};
}

#endif /* SRC_PLUGIN_HPP_ */
