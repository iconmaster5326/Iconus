/*
 * iif.cpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#include <sstream>
#include "iif.hpp"
#include "base64.hpp"

using namespace std;
using namespace iconus;

IIFMessage iconus::IIFMessage::makeEval(const std::string& tag, const std::string& content) {
	return IIFMessage(IIFType::EVAL, vector<string>({tag, content}));
}

IIFMessage iconus::IIFMessage::makeResult(const std::string& tag, const std::string& content) {
	return IIFMessage(IIFType::RESULT, vector<string>({tag, content}));
}

iconus::IIFMessage::IIFMessage(const std::string& packet) {
	istringstream scanner(packet);
	string cmd; scanner >> cmd;
	
	if (cmd == "eval") {
		type = IIFType::EVAL;
		string tag; scanner >> tag;
		args.push_back(tag);
		string content; scanner >> content;
		args.push_back(base64decode(content));
	} else if (cmd == "result") {
		type = IIFType::RESULT;
		string tag; scanner >> tag;
		args.push_back(tag);
		string content; scanner >> content;
		args.push_back(base64decode(content));
	} else {
		throw exception();
	}
}

std::string iconus::IIFMessage::getTag() {
	return args[0];
}

std::string iconus::IIFMessage::getContent() {
	return args[1];
}

iconus::IIFMessage::operator std::string() {
	ostringstream sb;
	
	switch (type) {
	case IIFType::EVAL: {
		sb << "eval";
		sb << " ";
		sb << args[0];
		sb << " ";
		sb << base64encode(args[1]);
	} break;
	case IIFType::RESULT: {
		sb << "result";
		sb << " ";
		sb << args[0];
		sb << " ";
		sb << base64encode(args[1]);
	} break;
	}
	
	sb << endl;
	return sb.str();
}
