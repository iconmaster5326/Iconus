/*
 * plugin.cpp
 *
 *  Created on: Jan 21, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "classes.hpp"
#include "error.hpp"
#include "base64.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>

#include <sys/capability.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>

using namespace std;
using namespace iconus;
using namespace boost::filesystem;

extern "C" string iconus_getName() {
	return "Standard Library";
}

extern "C" void iconus_initGlobalScope(GlobalScope& scope) {
	////////////////////////////
	// functions
	////////////////////////////
	
	using Arg = Function::Arg;
	
	scope.vars["echo"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"input", "", "",
			{}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input;
			}
	));
	
	scope.vars["list"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "",
			{}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassList::create(varargs.begin(), varargs.end());
			}
	));
	
	scope.vars["apply"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "args", "flags",
			{Arg("fn")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return args["fn"]->execute(session, scope, input, varargs, varflags);
			}
	));
	
	scope.vars["get"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->getField(session, args["k"]);
			}
	));
	
	scope.vars["set"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("k"), Arg("v")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		input->setField(session, args["k"], args["v"]);
		return input;
			}
	));
	
	scope.vars["local"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{Arg("v")}, {},
			[](auto& session, auto& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		string name = ClassString::value(session, args["v"]);
		scope.setLocal(name, input);
		return input;
			}
	));
	
	scope.vars["vars"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"", "", "",
			{}, {},
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
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
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return input->adapt(session, &ClassBool::INSTANCE);
			}
	));
	
	scope.vars["=="] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"a", "", "",
			{Arg("b")}, {},
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassBool::create(args["a"]->equals(args["b"]));
			}
	));
	
	scope.vars["get-class"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"i", "", "",
			{}, {},
			[](auto& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		return ClassClass::create(input->clazz);
			}
	));
	
	scope.vars["ls"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
			"dir", "", "",
			{}, {},
			[](Session& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		try {
			Object* result = ClassList::create();
			Deque<Object*>& items = ClassList::value(session, result);
			session.user.doAsUser([&]() {
				path p{input == &ClassNil::NIL ? "." : ClassString::value(session, input)};
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
			[](Session& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		try {
			Object* result;
			session.user.doAsUser([&]() {
				path p{ClassString::value(session, input)};
				if (exists(status(p))) {
					result = session.cat(p.string());
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
			[](Session& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		Object* result;
		session.user.doAsUser([&]() {
			int link[2];
			pid_t pid;
			int status;
			
			status = pipe(link);
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
					cerr << "system: error in initializing capabilities: " << strerror(errno) << endl;
					exit(1);
				}
				status = cap_clear(caps);
				if (status < 0) {
					cerr << "system: error in clearing capabilities: " << strerror(errno) << endl;
					exit(1);
				}
				status = cap_set_proc(caps);
				if (status < 0) {
					cerr << "system: error in setting capabilities: " << strerror(errno) << endl;
					exit(1);
				}
				cap_free(caps);
				
				dup2(link[1], STDOUT_FILENO);
				dup2(link[1], STDERR_FILENO);
				
			    close(link[0]);
			    close(link[1]);
			    
			    const char* argv[varargs.size()+2];
			    argv[0] = ClassString::value(session, args["name"]).c_str();
			    int i = 1;
			    for (Object* arg : varargs) {
			    	argv[i] = ClassString::value(session, arg).c_str();
			    	i++;
			    }
			    argv[varargs.size()+1] = nullptr;
			    
			    execvp(argv[0], (char* const*) argv);
			    cerr << "system: error in child process: " << strerror(errno) << endl;
			    exit(1);
			} else {
				// parent: wait for child to complete and get output
				close(link[1]);
				
				const size_t nBuffer = 1024;
				char buffer[nBuffer];
				string s;
				
				int bytesRead;
				do {
					bytesRead = read(link[0], buffer, nBuffer);
					s += string(buffer, bytesRead);
				} while (bytesRead > 0);
				
				if (bytesRead < 0) {
					throw Error("system: could not read output: "+string(strerror(errno)));
				}
				
				waitpid(pid, nullptr, 0);
				result = ClassString::create(s);
			}
		});
		return result;
			}
	));
	
	if (User::IS_ROOT) { // some commands, like login, are useless if we're not root
		scope.vars["login"] = new Object(&ClassManagedFunction::INSTANCE, new ClassManagedFunction::Instance(
				"", "", "",
				{Arg("user"),Arg("pass")}, {},
				[](Session& session, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
			session.user = User(ClassString::value(session, args["user"]), ClassString::value(session, args["pass"]));
			return &ClassNil::NIL;
				}
		));
	}
}

extern "C" void iconus_initSession(Session& session) {
	////////////////////////////
	// word parsers
	////////////////////////////
	
	session.parsers.emplace_back([](Session& session, const std::string& word) {
		return word == "nil";
	},[](Session& session, string word) {
		return &ClassNil::NIL;
	});
	
	session.parsers.emplace_back([](Session& session, const std::string& word) {
		return word == "true";
	},[](Session& session, string word) {
		return &ClassBool::TRUE;
	});
	
	session.parsers.emplace_back([](Session& session, const std::string& word) {
		return word == "false";
	},[](Session& session, string word) {
		return &ClassBool::FALSE;
	});
	
	session.parsers.emplace_back([](Session& session, const std::string& word) {
		try {
			stod(word);
			return true;
		} catch (const invalid_argument& ex) {
			return false;
		} catch (const out_of_range& ex) {
			throw Error("Numeric constant out of range: "+word);
		}
	},[](Session& session, string word) {
		return new Object(&ClassNumber::INSTANCE, stod(word));
	});
	
	////////////////////////////
	// renderers
	////////////////////////////
	
	session.renderers.emplace_back("image", [](Session& session, Object* ob) {
		return ob->clazz == &ClassImage::INSTANCE;
	}, [](Session& session, Object* ob) {
		return "<img src=\""+ClassImage::value(session, ob)+"\">";
	});
	
	session.renderers.emplace_back("numbered list", [](Session& session, Object* ob) {
		return ob->clazz == &ClassList::INSTANCE;
	}, [](Session& session, Object* ob) {
		ostringstream sb;
		sb << "<ol>";
		
		Deque<Object*>& items = *((Deque<Object*>*)ob->value.asPtr);
		for (Object* item : items) {
			sb << "<li>" << session.render(item) << "</li>";
		}
		
		sb << "</ol>";
		return sb.str();
	});
	
	session.renderers.emplace_back("error", [](Session& session, Object* ob) {
		return ob->clazz == &ClassError::INSTANCE;
	}, [](Session& session, Object* ob) {
		ostringstream sb;
		sb << "<div style=\"color: red;\"><b>error:</b> ";
		
		Object* what = (Object*) ob->value.asPtr;
		sb << session.render(what);
		
		sb << "</div>";
		return sb.str();
	});
	
	session.renderers.emplace_back("raw string", [](Session& session, Object* ob) {
		return true;
	}, [](Session& session, Object* ob) {
		return ob->toString(session);
	});
	
	////////////////////////////
	// adaptors
	////////////////////////////
	
	session.adaptors[&ClassNumber::INSTANCE] = {};
	session.adaptors[&ClassString::INSTANCE] = {};
	session.adaptors[&ClassBool::INSTANCE] = {};
	
	session.adaptors[&ClassNumber::INSTANCE][&ClassString::INSTANCE] = [](Session& session, Object* from) {
		double value = ClassNumber::value(session, from);
		return ClassString::create(to_string(value));
	};
	
	session.adaptors[&ClassNumber::INSTANCE][&ClassBool::INSTANCE] = [](Session& session, Object* from) {
		double value = ClassNumber::value(session, from);
		if (value == 0.0) return &ClassBool::FALSE; else return &ClassBool::TRUE;
	};
	
	session.adaptors[&ClassString::INSTANCE][&ClassNumber::INSTANCE] = [](Session& session, Object* from) {
		const string& value = ClassString::value(session, from);
		return ClassNumber::create(stod(value));
	};
	
	////////////////////////////
	// cat handlers
	////////////////////////////
	session.catHandlers.emplace_back([](Session& session, const string& file) {
		return file.substr(file.size()-4) == ".png";
	}, [](Session& session, const string& file) {
		std::ifstream in{file, ios::in | ios::binary};
		if (!in.eof() && in.fail()) throw Error("cat: could not open file '"+file+"'");
		Base64Buffer buf{Base64Buffer::fromStream(in)};
		if (!in.eof() && in.fail()) throw Error("cat: could not read file '"+file+"'");
		return ClassImage::create("data:image/png;base64,"+base64encode(buf));
	});
	
	session.catHandlers.emplace_back([](Session& session, const string& file) {
		return true;
	}, [](Session& session, const string& file) {
		std::ifstream in{file};
		if (in.fail()) throw Error("cat: could not open file '"+file+"'");
		string result{static_cast<std::stringstream const&>(std::stringstream() << in.rdbuf()).str()};
		if (in.fail()) throw Error("cat: could not read file '"+file+"'");
		return ClassString::create(result);
	});
}
