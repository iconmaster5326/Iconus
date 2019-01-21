# vars
ifeq ($(OS),Windows_NT)
EXE_SUFFIX := .exe
endif

CXX_FILES := $(wildcard src/*.cpp)
HXX_FILES := $(wildcard src/*.hpp)
O_FILES := $(patsubst src/%.cpp,build/%.o,$(CXX_FILES))
D_FILES := $(patsubst src/%.cpp,build/%.d,$(CXX_FILES))
EXE_FILES := iconus test/test_parser

PLUGINS := $(wildcard plugins/*)
PLUGIN_DLS := $(patsubst plugins/%,%.icolib,$(PLUGINS))

CXXFLAGS := -g -std=c++14 -I. -Isrc -ISimple-Web-Server -ISimple-WebSocket-Server
LINKFLAGS := -lcrypto -lpthread -lboost_system -lboost_thread -lgc -lgccpp -ldl
PLUGIN_FLAGS := -fPIC -shared

CURL := curl
TAR := tar

# executables
# (main exe must be at top, so it's default)
iconus$(EXE_SUFFIX): $(O_FILES) $(PLUGIN_DLS)
	$(CXX) $(CXXFLAGS) -o iconus $(O_FILES) $(LINKFLAGS)

# source files
$(D_FILES): | build/index.cxx build Simple-Web-Server Simple-WebSocket-Server json.hpp
$(D_FILES): build/%.d: src/%.cpp
	$(CXX) -MM -MT '$(patsubst %.d,%.o,$@) $@' $(CXXFLAGS) $< > $@
include $(D_FILES)

$(O_FILES): | build Simple-Web-Server Simple-WebSocket-Server
$(O_FILES): build/%.o: build/%.d
	$(CXX) $(CXXFLAGS) -c -o $@ $(patsubst build/%.d,src/%.cpp,$<)

build:
	mkdir build

# tests
TEST_O_FILES := $(filter-out build/main.o, $(O_FILES))

test/test_lexer$(EXE_SUFFIX): test/test_lexer.cpp $(TEST_O_FILES)
	$(CXX) $(CXXFLAGS) -o test/test_lexer test/test_lexer.cpp $(TEST_O_FILES) $(LINKFLAGS)

test/test_parser$(EXE_SUFFIX): test/test_parser.cpp $(TEST_O_FILES)
	$(CXX) $(CXXFLAGS) -o test/test_parser test/test_parser.cpp $(TEST_O_FILES) $(LINKFLAGS)

# embedded files
build/index.cxx: src/index.html | build
	xxd -i src/index.html > build/index.cxx

# plugins
$(PLUGIN_DLS): | $(HXX_FILES) json.hpp
$(PLUGIN_DLS): %.icolib: plugins/%
	$(CXX) $(PLUGIN_FLAGS) $(CXXFLAGS) -o $@ $(<)/*.cpp $(LINKFLAGS)

# dependencies
SWS_VER := v3.0.0-rc3

Simple-Web-Server.tar.gz:
	$(CURL) "https://gitlab.com/eidheim/Simple-Web-Server/-/archive/$(SWS_VER)/Simple-Web-Server-$(SWS_VER).tar.gz" > Simple-Web-Server.tar.gz

Simple-Web-Server: Simple-Web-Server.tar.gz
	$(TAR) -xvzf Simple-Web-Server.tar.gz
	mv Simple-Web-Server-$(SWS_VER) Simple-Web-Server

json.hpp:
	$(CURL) -L "https://github.com/nlohmann/json/releases/download/v3.5.0/json.hpp" > json.hpp

SWSS_VER := v2.0.0-rc4

Simple-WebSocket-Server.tar.gz:
	$(CURL) "https://gitlab.com/eidheim/Simple-WebSocket-Server/-/archive/$(SWSS_VER)/Simple-WebSocket-Server-$(SWSS_VER).tar.gz" > Simple-WebSocket-Server.tar.gz

Simple-WebSocket-Server: Simple-WebSocket-Server.tar.gz
	$(TAR) -xvzf Simple-WebSocket-Server.tar.gz
	mv Simple-WebSocket-Server-$(SWSS_VER) Simple-WebSocket-Server

# phony rules
all: $(patsubst %,%$(EXE_SUFFIX),$(EXE_FILES))

clean:
	rm -rf build

spotless: clean
	rm -rf $(EXE_FILES) $(patsubst %,%.exe,$(EXE_FILES))

.PHONY: clean spotless all
