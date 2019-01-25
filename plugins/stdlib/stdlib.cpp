/*
 * plugin.cpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#include "std_classes.hpp"

#include "session.hpp"
#include "error.hpp"
#include "base64.hpp"
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

using namespace std;
using namespace iconus;
using namespace boost::filesystem;

#include "build/stdlib/header.cxx"
static string header((const char*)header_html, header_html_len);

extern "C" string iconus_getName() {
	return "Standard Library";
}

extern "C" string iconus_initHTML() {
	return header;
}

extern "C" void iconus_initGlobalScope(GlobalScope& scope) {
	////////////////////////////
	// functions
	////////////////////////////
	
	using Arg = Function::Arg;
	
	scope.vars["echo"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"input", "", "",
			{}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input;
			}
	));
	
	scope.vars["list"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "",
			{}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassList::create(varargs.begin(), varargs.end());
			}
	));
	
	scope.vars["apply"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "flags",
			{Arg("fn")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return args["fn"]->execute(exe, scope, input, varargs, varflags);
			}
	));
	
	scope.vars["get"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->getField(exe, args["k"]);
			}
	));
	
	scope.vars["set"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k"), Arg("v")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		input->setField(exe, args["k"], args["v"]);
		return input;
			}
	));
	
	scope.vars["local"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("v")}, {},
			[](auto& exe, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		string name = ClassString::value(exe, args["v"]);
		scope.setLocal(name, input);
		return input;
			}
	));
	
	scope.vars["vars"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "", "",
			{}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		Deque<Object*> result;
		for (auto& pair : scope.vars) {
			result.push_back(ClassString::create(pair.first));
		}
		return ClassList::create(result);
			}
	));
	
	scope.vars["bool"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->adapt(exe, &ClassBool::INSTANCE);
			}
	));
	
	scope.vars["=="] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"a", "", "",
			{Arg("b")}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassBool::create(args["a"]->equals(args["b"]));
			}
	));
	
	scope.vars["get-class"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{}, {},
			[](auto& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassClass::create(input->clazz);
			}
	));
	
	scope.vars["ls"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"dir", "", "",
			{}, {},
			[](Execution& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		try {
			Object* result = ClassList::create();
			Deque<Object*>& items = ClassList::value(exe, result);
			exe.session.user.doAsUser([&]() {
				path p{input == &ClassNil::NIL ? "." : ClassString::value(exe, input)};
				for (directory_iterator it{p}; it != directory_iterator{}; it++) {
					items.push_back(ClassString::create(it->path().string()));
				}
			});
			return result;
		} catch (const filesystem_error& e) {
			throw Error(e.what());
		}
			}
	));
	
	scope.vars["cat"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"file", "", "",
			{}, {},
			[](Execution& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		try {
			Object* result;
			exe.session.user.doAsUser([&]() {
				path p{ClassString::value(exe, input)};
				if (exists(status(p))) {
					result = exe.cat(p.string());
				} else {
					throw Error("cat: file '"+p.string()+"' not found");
				}
			});
			return result;
		} catch (const filesystem_error& e) {
			throw Error(e.what());
		}
			}
	));
	
	scope.vars["system"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "",
			{Arg("name")}, {},
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
	
	if (User::IS_ROOT) { // some commands, like login, are useless if we're not root
		scope.vars["login"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
				"", "", "",
				{Arg("user"),Arg("pass")}, {},
				[](Execution& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
			exe.session.user = User(ClassString::value(exe, args["user"]), ClassString::value(exe, args["pass"]));
			return &ClassNil::NIL;
				}
		));
	}
}

extern "C" void iconus_initSession(Execution& exe) {
	////////////////////////////
	// word parsers
	////////////////////////////
	
	exe.session.parsers.emplace_back([](Execution& exe, const std::string& word) {
		return word == "nil";
	},[](Execution& exe, string word) {
		return &ClassNil::NIL;
	});
	
	exe.session.parsers.emplace_back([](Execution& exe, const std::string& word) {
		return word == "true";
	},[](Execution& exe, string word) {
		return &ClassBool::TRUE;
	});
	
	exe.session.parsers.emplace_back([](Execution& exe, const std::string& word) {
		return word == "false";
	},[](Execution& exe, string word) {
		return &ClassBool::FALSE;
	});
	
	exe.session.parsers.emplace_back([](Execution& exe, const std::string& word) {
		try {
			stod(word);
			return true;
		} catch (const invalid_argument& ex) {
			return false;
		} catch (const out_of_range& ex) {
			throw Error("Numeric constant out of range: "+word);
		}
	},[](Execution& exe, string word) {
		return new Object(&ClassNumber::INSTANCE, stod(word));
	});
	
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
	
	exe.session.renderers.emplace_back("cat result", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassRawString::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		return "<pre>" + escapeHTML(ClassRawString::value(exe, ob)) + "</pre>";
	});
	
	exe.session.renderers.emplace_back("image", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassImage::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		return "<img src=\""+ClassImage::value(exe, ob)+"\">";
	});
	
	exe.session.renderers.emplace_back("numbered list", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassList::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		ostringstream sb;
		sb << "<ol>";
		
		Deque<Object*>& items = *((Deque<Object*>*)ob->value.asPtr);
		for (Object* item : items) {
			sb << "<li>" << exe.render(item) << "</li>";
		}
		
		sb << "</ol>";
		return sb.str();
	});
	
	exe.session.renderers.emplace_back("error", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassError::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		ostringstream sb;
		sb << "<div style=\"color: red;\"><b>error:</b> ";
		
		Object* what = (Object*) ob->value.asPtr;
		sb << exe.render(what);
		
		sb << "</div>";
		return sb.str();
	});
	
	exe.session.renderers.emplace_back("raw string", [](Execution& exe, Object* ob) {
		return true;
	}, [](Execution& exe, Object* ob) {
		return escapeHTML(ob->toString(exe));
	});
	
	////////////////////////////
	// adaptors
	////////////////////////////
	
	exe.session.adaptors[&ClassNumber::INSTANCE] = {};
	exe.session.adaptors[&ClassString::INSTANCE] = {};
	exe.session.adaptors[&ClassBool::INSTANCE] = {};
	
	exe.session.adaptors[&ClassNumber::INSTANCE][&ClassString::INSTANCE] = [](Execution& exe, Object* from) {
		double value = ClassNumber::value(exe, from);
		return ClassString::create(to_string(value));
	};
	
	exe.session.adaptors[&ClassNumber::INSTANCE][&ClassBool::INSTANCE] = [](Execution& exe, Object* from) {
		double value = ClassNumber::value(exe, from);
		if (value == 0.0) return &ClassBool::FALSE; else return &ClassBool::TRUE;
	};
	
	exe.session.adaptors[&ClassString::INSTANCE][&ClassNumber::INSTANCE] = [](Execution& exe, Object* from) {
		const string& value = ClassString::value(exe, from);
		return ClassNumber::create(stod(value));
	};
	
	exe.session.adaptors[&ClassString::INSTANCE][&ClassRawString::INSTANCE] = [](Execution& exe, Object* from) {
		return new Object(&ClassRawString::INSTANCE, from->value.asPtr);
	};
	exe.session.adaptors[&ClassRawString::INSTANCE][&ClassString::INSTANCE] = [](Execution& exe, Object* from) {
		return new Object(&ClassString::INSTANCE, from->value.asPtr);
	};
	
	////////////////////////////
	// cat handlers
	////////////////////////////
	exe.session.catHandlers.emplace_back([](Execution& exe, const string& file) {
		return file.substr(file.size()-4) == ".png";
	}, [](Execution& exe, const string& file) {
		std::ifstream in{file, ios::in | ios::binary};
		if (!in.eof() && in.fail()) throw Error("cat: could not open file '"+file+"'");
		Base64Buffer buf{Base64Buffer::fromStream(in)};
		if (!in.eof() && in.fail()) throw Error("cat: could not read file '"+file+"'");
		return ClassImage::create("data:image/png;base64,"+base64encode(buf));
	});
	
	exe.session.catHandlers.emplace_back([](Execution& exe, const string& file) {
		return true;
	}, [](Execution& exe, const string& file) {
		std::ifstream in{file};
		if (in.fail()) throw Error("cat: could not open file '"+file+"'");
		string result{static_cast<std::stringstream const&>(std::stringstream() << in.rdbuf()).str()};
		if (in.fail()) throw Error("cat: could not read file '"+file+"'");
		return ClassRawString::create(result);
	});
}
