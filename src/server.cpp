/*
 * server.cpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#include "server.hpp"
#include "session.hpp"
#include "plugin.hpp"

#include <iostream>
#include <boost/uuid/uuid_io.hpp>

#include "server_http.hpp"
#include "server_ws.hpp"
#include "json.hpp"

using namespace std;
using namespace boost::uuids;
using namespace SimpleWeb;
using HttpServer = Server<HTTP>; // TODO: use HTTPS and WSS
using WsServer = SocketServer<WS>;

namespace iconus {
	static void find_and_replace(string& s, const string& from, const string& to) {
		size_t n = s.find(from, 0);
		while (n != string::npos) {
			s.replace(n, from.size(), to);
			n = s.find(from, n + to.size());
		}
	}
	
	static Map<string, function<void(nlohmann::json&)>> messageHandlers;

	void startServer(const std::string& addr, unsigned short port, const std::string& html) {
		HttpServer server;
		server.config.address = addr;
		server.config.port = port;
		
		server.default_resource["GET"] = [&html](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
			cout << "Serving HTML..." << endl;
			
			string processedHTML{html};
			for (Plugin& plugin : Plugin::plugins) {
				find_and_replace(processedHTML, "@HEADER@", "\n" + plugin.initHTML + "\n@HEADER@");
			}
			find_and_replace(processedHTML, "@HEADER@", "");
			
			response->write(StatusCode::success_ok, processedHTML);
		};
		
		server.on_upgrade = [](unique_ptr<HTTP>& socket, shared_ptr<HttpServer::Request> request) {
			cout << "Upgrade request obtained!" << endl;
			WsServer* wsServer = new WsServer();
			Session* session = new Session();
			
			// specify
			auto& endpoint = wsServer->endpoint[".*"];
			
			endpoint.on_message = [session](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::InMessage> in_message) {
				string input = in_message->string();
				cout << "GOT: " << input << endl;
				nlohmann::json message = nlohmann::json::parse(input);
				string type = message["type"].get<string>();
				if (type == "eval") {
					string tag = message["tag"].get<string>();
					Execution* exe = new Execution(*session, string_generator()(tag));
					
					exe->sendMessage = [exe,connection](uuid& id, auto& map) {
						if (find_if(exe->idsRendered.begin(), exe->idsRendered.end(), [&id](uuid* v) {
							return *v == id;
						}) != exe->idsRendered.end()) {
							nlohmann::json message = {
									{"type", "message"},
									{"tag", to_string(id)},
							};
							
							for (auto& pair : map) {
								message[pair.first] = pair.second;
							}
							
							connection->send(message.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
						}
					};
					
					exe->getMessage = [exe,connection](uuid& id, auto& map) {
						messageHandlers[to_string(id)] = [&](nlohmann::json& message) {
							if (find_if(exe->idsRendered.begin(), exe->idsRendered.end(), [&id](uuid* v) {
								return *v == id;
							}) != exe->idsRendered.end()) {
								for (auto& pair : message.get<nlohmann::json::object_t>()) {
									map[pair.first] = pair.second.get<string>();
								}
								
								messageHandlers.erase(to_string(id));
							}
						};
					};
					
					Object* result = session->evaluate(message["command"].get<string>(), *exe);
					
					nlohmann::json response = {
							{"type", "result"},
							{"tag", tag},
							{"result", exe->render(result)},
					};
					connection->send(response.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
					
					session->addHistory(message["command"].get<string>(), result);
					exe->completed = true;
				} else if (type == "message") {
					string tag = message["tag"].get<string>();
					auto it = messageHandlers.find(tag);
					if (it == messageHandlers.end()) {
						cout << "WARNING: Incoming message to non-registered object " << tag << " recieved" << endl;
					} else {
						it->second(message);
					}
				} else {
					cout << "WARNING: unknown command type "+type+" recieved";
				}
			};
			
			endpoint.on_close = [wsServer,session](shared_ptr<WsServer::Connection> connection, int code, const string& reason) {
				cout << "WebSocket closed. Reason: " << reason << endl;
				session->closed = true;
				delete wsServer;
			};
			
			// connect
			auto connection = make_shared<SocketServer<WS>::Connection>(move(socket));
			connection->method = move(request->method);
			connection->path = move(request->path);
			connection->query_string = move(request->query_string);
			connection->http_version = move(request->http_version);
			connection->header = move(request->header);
			connection->remote_endpoint = move(*request->remote_endpoint);
			
			wsServer->upgrade(connection);
			cout << "Upgrade request completed." << endl;
		};
		
		cout << "Server started." << endl;
		server.start();
	}
}
