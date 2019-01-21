/*
 * session.hpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#ifndef SRC_SESSION_HPP_
#define SRC_SESSION_HPP_

#include "program.hpp"
#include "render.hpp"

#include <string>

#include <functional>

namespace iconus {
	using Adaptor = std::function<Object*(Session&, Object*)>;
	
	class WordParser {
	public:
		using Filter = std::function<bool(Session&, const std::string&)>;
		using Handler = std::function<Object*(Session&, const std::string&)>;
		
		inline WordParser(Filter filter, Handler handler) : filter(filter), handler(handler) {}
		
		Filter filter;
		Handler handler;
	};
	
	class Session {
	public:
		Session();
		
		Object* evaluate(const std::string& input);
		std::string render(Object* object);
		Object* parseWord(std::string word);
		Adaptor getAdaptor(Class* from, Class* to);
		
		Scope globalScope;
		Vector<Renderer> renderers;
		Vector<WordParser> parsers;
		Map<Class*, Map<Class*, Adaptor>> adaptors;
	private:
		void addGlobalScope();
		void addDefaultRenderers();
		void addDefaultWordParsers();
	};
}

#endif /* SRC_SESSION_HPP_ */
