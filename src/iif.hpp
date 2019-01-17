/*
 * iif.hpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 *   
 *  Iconus Interchange Format (IIF) is the low-level protocol used to communicate between client and server.
 */

#ifndef SRC_IIF_HPP_
#define SRC_IIF_HPP_

#include <vector>
#include <string>

namespace iconus {
	enum class IIFType {
		EVAL,
		RESULT,
	};

	class IIFMessage {
	public:
		static IIFMessage makeEval(const std::string& tag, const std::string& content);
		static IIFMessage makeResult(const std::string& tag, const std::string& content);
		
		IIFMessage(const std::string& packet);
		
		IIFType type;
		std::string getTag();
		std::string getContent();
		operator std::string();
	private:
		inline IIFMessage(IIFType type, std::vector<std::string> args) : type(type), args(args) {}
		
		std::vector<std::string> args;
	};
}

#endif /* SRC_IIF_HPP_ */
