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
#include "op.hpp"

#include <string>
#include <functional>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace iconus {
	class Session; class Execution;
	
	using Adaptor = std::function<Object*(Execution&, Object*)>;
	
	class CatHandler {
	public:
		using Filter = std::function<bool(Execution&, const std::string&)>;
		using Handler = std::function<Object*(Execution&, const std::string&)>;
		
		inline CatHandler(Filter filter, Handler handler) : filter(filter), handler(handler) {}
		
		Filter filter;
		Handler handler;
	};
	
	class WordParser {
	public:
		using Filter = std::function<bool(Execution&, const std::string&)>;
		using Handler = std::function<Object*(Execution&, const std::string&)>;
		
		inline WordParser(Filter filter, Handler handler) : filter(filter), handler(handler) {}
		
		Filter filter;
		Handler handler;
	};
	
	class Renderer {
	public:
		using Filter = std::function<bool(Execution&, Object*)>;
		using Handler = std::function<std::string(Execution&, Object*)>;
		
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
	
	class Execution : public gc {
	public:
		using MessageHandler = std::function<void(boost::uuids::uuid&, Map<std::string, std::string>&)>;
		
		inline Execution(Session& session) : session(session), tag(boost::uuids::nil_generator()()) {}
		inline Execution(Session& session, boost::uuids::uuid tag) : session(session), tag(tag) {}
		
		Execution(const Execution&) = delete;
		
		std::string render(Object* object);
		Object* parseWord(std::string word);
		Adaptor getAdaptor(Class* from, Class* to);
		Object* cat(const std::string& file);
		
		Session& session;
		boost::uuids::uuid tag;
		MessageHandler sendMessage = nullptr;
		MessageHandler getMessage = nullptr;
		bool completed = false;
		Set<boost::uuids::uuid*> idsRendered;
	};
	
	class HistoryItem {
	public:
		std::string input;
		Object* output;
		
		inline HistoryItem(const std::string& input, Object* output) : input{input}, output{output} {}
	};
	
	class Session { // TODO: inherit gc; it seems there's a bug in libgc causing all Sessions to get garbage collected early
	public:
		Session();
		Session(const Session&) = delete;
		
		Object* evaluate(const std::string& input, Execution& exe);
		void addHistory(const std::string& input, Object* output);
		
		User user;
		Scope sessionScope;
		Vector<Renderer> renderers;
		Vector<WordParser> parsers;
		Map<Class*, Map<Class*, Adaptor>> adaptors;
		Vector<CatHandler> catHandlers;
		bool closed;
		unsigned maxHistory = 100;
		Vector<HistoryItem> history;
	};
}

#endif /* SRC_SESSION_HPP_ */
