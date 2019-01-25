/*
 * session.hpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#ifndef SRC_SESSION_HPP_
#define SRC_SESSION_HPP_

#include "program.hpp"
#include "user.hpp"

#include <string>
#include <functional>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace iconus {
	class Session;
	
	using Adaptor = std::function<Object*(Session&, Object*)>;
	
	class CatHandler {
	public:
		using Filter = std::function<bool(Session&, const std::string&)>;
		using Handler = std::function<Object*(Session&, const std::string&)>;
		
		inline CatHandler(Filter filter, Handler handler) : filter(filter), handler(handler) {}
		
		Filter filter;
		Handler handler;
	};
	
	class WordParser {
	public:
		using Filter = std::function<bool(Session&, const std::string&)>;
		using Handler = std::function<Object*(Session&, const std::string&)>;
		
		inline WordParser(Filter filter, Handler handler) : filter(filter), handler(handler) {}
		
		Filter filter;
		Handler handler;
	};
	
	class Renderer {
	public:
		using Filter = std::function<bool(Session&, Object*)>;
		using Handler = std::function<std::string(Session&, Object*)>;
		
		inline Renderer(const std::string& name, Filter filter, Handler handler) : name(name), filter(filter), handler(handler) {}
		
		std::string name;
		Filter filter;
		Handler handler;
	};
	
	class GlobalScope : public Scope {
	public:
		static GlobalScope INSTANCE;
		bool canSet(const std::string& name) override;
	};
	
	class Execution {
	public:
		inline Execution(Session& session) : session(session), tag(boost::uuids::nil_generator()()) {}
		inline Execution(Session& session, boost::uuids::uuid tag) : session(session), tag(tag) {}
		
		Session& session;
		boost::uuids::uuid tag;
	};
	
	class Session {
	public:
		Session();
		
		Object* evaluate(const std::string& input, Execution& exe);
		std::string render(Object* object);
		Object* parseWord(std::string word);
		Adaptor getAdaptor(Class* from, Class* to);
		Object* cat(const std::string& file);
		
		Execution defaultExecution;
		User user;
		Scope sessionScope;
		Vector<Renderer> renderers;
		Vector<WordParser> parsers;
		Map<Class*, Map<Class*, Adaptor>> adaptors;
		Vector<CatHandler> catHandlers;
	};
}

#endif /* SRC_SESSION_HPP_ */
