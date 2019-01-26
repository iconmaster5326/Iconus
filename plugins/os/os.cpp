/*
 * plugin.cpp
 *
 *  Created on: Jan 25, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "error.hpp"
#include "util.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <cerrno>
#include <cstdlib>
#include <thread>
#include <boost/uuid/uuid_io.hpp>

#include <sys/capability.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include "../os/lib_classes.hpp"

using namespace std;
using namespace iconus;
using namespace boost::filesystem;

#include "build/os/header.cxx"
static string header((const char*)header_html, header_html_len);

extern "C" string iconus_getName() {
	return "OS Integration";
}

extern "C" string iconus_initHTML() {
	return header;
}

using Arg = Function::Arg;
constexpr Function::Role INPUT = Function::Role::INPUT;
constexpr Function::Role VARARG = Function::Role::VARARG;
constexpr Function::Role VARFLAG = Function::Role::VARFLAG;

extern "C" void iconus_initGlobalScope(GlobalScope& scope) {
	////////////////////////////
	// functions
	////////////////////////////
	scope.vars["system"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			{Arg("name"), Arg("args", VARARG)}, {},
			[](Execution& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		Object* result;
		exe.session.user.doAsUser([&]() {
			int stdoutLink[2];
			int stderrLink[2];
			int errorLink[2];
			int stdinLink[2];
			pid_t pid;
			int status;
			
			status = pipe(stdoutLink);
			if (status < 0) {
				throw Error("system: could not open pipe: "+string(strerror(errno)));
			}
			status = pipe(stderrLink);
			if (status < 0) {
				throw Error("system: could not open pipe: "+string(strerror(errno)));
			}
			status = pipe(errorLink);
			if (status < 0) {
				throw Error("system: could not open pipe: "+string(strerror(errno)));
			}
			status = pipe(stdinLink);
			if (status < 0) {
				throw Error("system: could not open pipe: "+string(strerror(errno)));
			}
			
			pid = fork();
			if (pid == -1) {
				throw Error("system: could not fork: "+string(strerror(errno)));
			} else if (pid == 0) {
				// child: drop all capabilities and run program
				int status;
				
				cap_t caps = cap_get_proc();
				if (!caps) {
					string errStr = "system: error in initializing capabilities: " + string(strerror(errno));
					write(errorLink[1], (const void*) errStr.c_str(), errStr.size());
					fsync(errorLink[1]);
					exit(1);
				}
				status = cap_clear(caps);
				if (status < 0) {
					string errStr = "system: error in clearing capabilities: " + string(strerror(errno));
					write(errorLink[1], (const void*) errStr.c_str(), errStr.size());
					fsync(errorLink[1]);
					exit(1);
				}
				status = cap_set_proc(caps);
				if (status < 0) {
					string errStr = "system: error in setting capabilities: " + string(strerror(errno));
					write(errorLink[1], (const void*) errStr.c_str(), errStr.size());
					fsync(errorLink[1]);
					exit(1);
				}
				cap_free(caps);
				
				dup2(stdinLink[0], STDIN_FILENO);
				dup2(stdoutLink[1], STDOUT_FILENO);
				dup2(stderrLink[1], STDERR_FILENO);
				
				close(stdinLink[0]); close(stdinLink[1]);
			    close(stdoutLink[0]); close(stdoutLink[1]);
			    close(stderrLink[0]); close(stderrLink[1]);
			    close(errorLink[0]);
			    
			    const char* argv[varargs.size()+2];
			    argv[0] = ClassString::value(exe, args["name"]).c_str();
			    int i = 1;
			    for (Object* arg : varargs) {
			    	argv[i] = ClassString::value(exe, arg).c_str();
			    	i++;
			    }
			    argv[varargs.size()+1] = nullptr;
			    
			    execvp(argv[0], (char* const*) argv);
			    
				string errStr = "system: error in running program: " + string(strerror(errno));
				write(errorLink[1], (const void*) errStr.c_str(), errStr.size());
				fsync(errorLink[1]);
			    exit(1);
			} else {
				// parent: wait for child to complete and get output
				close(stdinLink[0]);
				close(stdoutLink[1]);
				close(stderrLink[1]);
				close(errorLink[1]);
				
				ClassSystemOutput::Instance* output = new ClassSystemOutput::Instance();
				
				constexpr int readLineEOF = 0;
				constexpr int readLinePartial = 1;
				constexpr int readLineFull = 2;
				
				const auto readLine = [readLineEOF,readLinePartial,readLineFull](int fd, string& s) {
					int nRead;
					while (true) {
						char c;
						errno = 0;
						nRead = read(fd, &c, 1);
						
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							return readLinePartial;
						} else if (nRead <= 0) {
							return readLineEOF;
						} else if (c == '\n') {
							return readLineFull;
						} else {
							s += c;
						}
					}
				};
				
				fcntl(stdoutLink[0], F_SETFL, fcntl(stdoutLink[0], F_GETFL) | O_NONBLOCK);
				fcntl(stderrLink[0], F_SETFL, fcntl(stderrLink[0], F_GETFL) | O_NONBLOCK);
				
				result = ClassSystemOutput::create(output);
				thread outputThread([=]() {
					int retCode, status;
					string outS{};
					string errS{};
					
					const int nFds = 3;
					struct pollfd fds[nFds] {
						{stdoutLink[0], POLLIN, 0},
						{stderrLink[0], POLLIN, 0},
						{errorLink[0], POLLIN, 0},
					};
					
					Map<string,string> inputMap;
					exe.getMessage(output->id, inputMap);
					while (true) {
						status = poll(fds, nFds, 0);
						if (!inputMap.empty()) {
							auto doKill = inputMap.find("kill");
							if (doKill != inputMap.end()) {
								kill(pid, SIGKILL);
							} else {
								string& s = inputMap["input"];
								write(stdinLink[1], (const void*) s.c_str(), s.size());
								
								inputMap.clear();
								exe.getMessage(output->id, inputMap);
							}
						}
						if (status < 0) continue;
						
						if (fds[0].revents & POLLIN) { // stdoutLink
							int readLineRet;
							do {
								readLineRet = readLine(stdoutLink[0], outS);
								if (readLineRet != readLinePartial) {
									output->lines.emplace_back(false, outS);
									outS = "";
									
									Map<string,string> message{{"line",to_string(output->lines.size()-1)}, {"text", output->lines.back().text}};
									exe.sendMessage(output->id, message);
								}
							} while (readLineRet == readLineFull);
						} else if (fds[1].revents & POLLIN) { // stderrLink
							int readLineRet;
							do {
								readLineRet = readLine(stderrLink[0], errS);
								if (readLineRet != readLinePartial) {
									output->lines.emplace_back(true, errS);
									errS = "";
									
									Map<string,string> message{{"line",to_string(output->lines.size()-1)}, {"text", output->lines.back().text}, {"stderr", "true"}};
									exe.sendMessage(output->id, message);
								}
							} while (readLineRet == readLineFull);
						} else if (fds[2].revents & POLLIN) { // errorLink
							string s;
							int readLineRet;
							do {
								readLineRet = readLine(errorLink[0], s);
								if (readLineRet != readLinePartial) {
									Map<string,string> message{{"error", escapeHTML(s)}};
									exe.sendMessage(output->id, message);
									return;
								}
							} while (true);
						} else {
							pid_t isDone = waitpid(pid, &retCode, WNOHANG);
							if (isDone > 0) break;
						}
					}
					
					output->retCode = retCode;
					output->done = true;
					
					Map<string,string> message{{"done","true"}, {"code", to_string(retCode)}};
					exe.sendMessage(output->id, message);
				});
				outputThread.detach();
			}
		});
		return result;
			}
	));
	
	////////////////////////////
	// constants
	////////////////////////////
	scope.vars["<system-output>"] = ClassClass::create(&ClassSystemOutput::INSTANCE);
}

extern "C" void iconus_initSession(Execution& exe) {
	////////////////////////////
	// renderers
	////////////////////////////
	exe.session.renderers.emplace_back("system output", [](Execution& exe, Object* ob) {
			return ob->clazz == &ClassSystemOutput::INSTANCE;
		}, [](Execution& exe, Object* ob) {
			ClassSystemOutput::Instance& value = ClassSystemOutput::value(exe, ob);
			ostringstream sb;
			sb << "<div><img src onerror=\"onSystemOutputLoad('" << to_string(value.id) << "', this)\"><pre>";
			
			for (const auto& line : value.lines) {
				sb << "<div" << (line.isErr ? " style=\"color: red;\"" : "") << ">" << escapeHTML(line.text) << endl << "</div>";
			}
			
			sb << "</pre>";
			
			if (value.done) {
				sb << "(exited with code ";
				sb << value.retCode;
				sb << ")";
			} else {
				sb << "<div id=\"sysOutControlPanel\">";
				sb << "<i id=\"spinner\" class=\"fa fa-spinner fa-spin\" style=\"font-size:24px\"></i>";
				sb << "<input type=\"text\" id=\"systemInput\" />";
				sb << "<button type=\"button\" id=\"sysOutKill\" onclick=\"onSystemOutputKill('" << to_string(value.id) << "')\">Kill</button>";
				sb << "</div>";
			}
			
			sb << "</div>";
			return sb.str();
	});
}
