/*
 * server.cpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#include "server.hpp"
#include "session.hpp"

#include <iostream>
#include "server_http.hpp"
#include "server_ws.hpp"
#include "json.hpp"

using namespace std;
using namespace boost::uuids;
using namespace SimpleWeb;
using HttpServer = Server<HTTP>; // TODO: use HTTPS and WSS
using WsServer = SocketServer<WS>;

namespace iconus {
	/*
	static void find_and_replace(string& s, const string& from, const string& to) {
		size_t n = s.find(from, 0);
		while (n != string::npos) {
			s.replace(n, from.size(), to);
			n = s.find(from, n);
		}
	}
	*/

	void startServer(const std::string& addr, unsigned short port, const std::string& html) {
		HttpServer server;
		server.config.address = addr;
		server.config.port = port;
		
		server.default_resource["GET"] = [&html](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
			cout << "Serving HTML..." << endl;
			response->write(StatusCode::success_ok, html);
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
					Execution exe(*session, string_generator()(message["tag"].get<string>()));
					nlohmann::json response = {
							{"type", "result"},
							{"tag", message["tag"].get<string>()},
							{"result", exe.render(session->evaluate(message["command"].get<string>(), exe))},
					};
					connection->send(response.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
				}
			};
			
			endpoint.on_close = [wsServer,session](shared_ptr<WsServer::Connection> connection, int code, const string& reason) {
				cout << "WebSocket closed. Reason: " << reason << endl;
				delete session;
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
