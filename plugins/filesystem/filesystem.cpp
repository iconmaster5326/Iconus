/*
 * plugin.cpp
 *
 *  Created on: Jan 25, 2019
 *      Author: iconmaster
 */

#include "session.hpp"
#include "error.hpp"
#include "base64.hpp"
#include "util.hpp"
#include "lib_classes.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace iconus;
using namespace boost::filesystem;

#include "build/filesystem/header.cxx"
static string header((const char*)header_html, header_html_len);

extern "C" string iconus_getName() {
	return "Filesystem";
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
	scope.vars["ls"] = ClassManagedFunction::create(
			{Arg("dir", INPUT)}, {Arg("l", &ClassBool::FALSE)},
			[](Execution& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		try {
			Object* result;
			exe.session.user.doAsUser([&]() {
				path p;
				if (input == &ClassNil::NIL) {
					p = current_path();
				} else {
					Lock lock{input->mutex};
					p = ClassFile::value(exe, input);
				}
				
				if (args["l"]->truthy()) {
					result = ClassTable::create((initializer_list<string>) {"perms","hard-links","user","group","size","last-changed","name"});
					ClassTable::Instance& table = ClassTable::value(exe, result);
					
					for (directory_iterator it{p}; it != directory_iterator{}; it++) {
						table.rows.emplace_back((initializer_list<Object*>) {
								&ClassNil::NIL,
								ClassNumber::create((double) hard_link_count(it->path())),
								&ClassNil::NIL,
								&ClassNil::NIL,
								is_regular_file(it->path()) ?
										ClassNumber::create((double) file_size(it->path())) :
										&ClassNil::NIL,
								&ClassNil::NIL,
								ClassFile::create(it->path())
						});
					}
				} else {
					result = ClassList::create();
					Deque<Object*>& items = ClassList::value(exe, result);
					
					for (directory_iterator it{p}; it != directory_iterator{}; it++) {
						items.push_back(ClassFile::create(it->path()));
					}
				}
			});
			return result;
		} catch (const filesystem_error& e) {
			throw Error(string(e.what()).substr(string("boost::filesystem::directory_iterator::construct: ").size()));
		}
			}
	);
	
	scope.vars["cat"] = ClassManagedFunction::create(
			{Arg("file", INPUT)}, {},
			[](Execution& exe, Scope& scope, auto input, auto& args, auto& varargs, auto& varflags) {
		try {
			Object* result;
			exe.session.user.doAsUser([&]() {
				Lock lock{input->mutex};
				path& p = ClassFile::value(exe, input);
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
	);
	
	////////////////////////////
	// constants
	////////////////////////////
	scope.vars["<raw-string>"] = ClassClass::create(&ClassRawString::INSTANCE);
	scope.vars["<image>"] = ClassClass::create(&ClassImage::INSTANCE);
	scope.vars["<file>"] = ClassClass::create(&ClassFile::INSTANCE);
}

extern "C" void iconus_initSession(Execution& exe) {
	////////////////////////////
	// word parsers
	////////////////////////////
	
	////////////////////////////
	// renderers
	////////////////////////////
	exe.session.renderers.emplace_back("cat result", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassRawString::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		Lock lock{ob->mutex};
		return "<pre>" + escapeHTML(ClassRawString::value(exe, ob)) + "</pre>";
	});
	
	exe.session.renderers.emplace_back("image", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassImage::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		Lock lock{ob->mutex};
		return "<img src=\""+ClassImage::value(exe, ob)+"\">";
	});
	
	exe.session.renderers.emplace_back("filename", [](Execution& exe, Object* ob) {
		return ob->clazz == &ClassFile::INSTANCE;
	}, [](Execution& exe, Object* ob) {
		Lock lock{ob->mutex};
		path& p = ClassFile::value(exe, ob);
		string str;
		
		exe.session.user.doAsUser([&]() {
			string absStr = p.string();
			string curStr = current_path().string();
			
			if (curStr == absStr) {
				str = ".";
			} else if (curStr.size() < absStr.size() && absStr.substr(0, curStr.size()) == curStr) {
				str = absStr.substr(curStr.size()+1);
			} else {
				str = absStr;
			}
		});
		
		return "<a href=\"javascript:onFileClick('"+p.string()+"')\">"+escapeHTML(str)+"</a>";
	});
	
	////////////////////////////
	// adaptors
	////////////////////////////
	if (exe.session.adaptors.find(&ClassString::INSTANCE) == exe.session.adaptors.end())
		exe.session.adaptors[&ClassString::INSTANCE] = {};
	exe.session.adaptors[&ClassRawString::INSTANCE] = {};
	exe.session.adaptors[&ClassFile::INSTANCE] = {};
	
	exe.session.adaptors[&ClassString::INSTANCE][&ClassRawString::INSTANCE] = [](Execution& exe, Object* from) {
		Lock lock{from->mutex};
		return new Object(&ClassRawString::INSTANCE, from->value.asPtr);
	};
	exe.session.adaptors[&ClassRawString::INSTANCE][&ClassString::INSTANCE] = [](Execution& exe, Object* from) {
		Lock lock{from->mutex};
		return new Object(&ClassString::INSTANCE, from->value.asPtr);
	};
	
	exe.session.adaptors[&ClassFile::INSTANCE][&ClassString::INSTANCE] = [](Execution& exe, Object* from) {
		Lock lock{from->mutex};
		path& p = ClassFile::value(exe, from);
		return ClassString::create(p.string());
	};
	exe.session.adaptors[&ClassString::INSTANCE][&ClassFile::INSTANCE] = [](Execution& exe, Object* from) {
		Lock lock{from->mutex};
		path p{from->toString(exe)};
		return ClassFile::create(p);
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
