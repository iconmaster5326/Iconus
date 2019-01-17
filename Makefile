# vars
ifeq ($(OS),Windows_NT)
EXE_SUFFIX := .exe
endif

CXX_FILES := $(wildcard src/*.cpp)
HXX_FILES := $(wildcard src/*.hpp)
O_FILES := $(patsubst src/%.cpp,build/%.o,$(CXX_FILES))
D_FILES := $(patsubst src/%.cpp,build/%.d,$(CXX_FILES))
EXE_FILES := iconus

CXXFLAGS := -g -std=c++11 -I. -Isrc -ISimple-Web-Server -ISimple-WebSocket-Server
LINKFLAGS := -lcrypto -lpthread -lboost_system -lboost_thread

CURL := curl
TAR := tar

# executables
# (main exe must be at top, so it's default)
iconus$(EXE_SUFFIX): $(O_FILES)
	$(CXX) $(CXXFLAGS) -o iconus $(O_FILES) $(LINKFLAGS)

# source files
$(D_FILES): build/index.cxx | build Simple-Web-Server Simple-WebSocket-Server
$(D_FILES): build/%.d: src/%.cpp
	$(CXX) -MM -MT '$(patsubst %.d,%.o,$@) $@' $(CXXFLAGS) $< > $@
include $(D_FILES)

$(O_FILES): | build Simple-Web-Server Simple-WebSocket-Server
$(O_FILES): build/%.o: build/%.d
	$(CXX) $(CXXFLAGS) -c -o $@ $(patsubst build/%.d,src/%.cpp,$<)

build:
	mkdir build

# embedded files
build/index.cxx: src/index.html | build
	xxd -i src/index.html > build/index.cxx

# dependencies
SWS_VER := v3.0.0-rc3

Simple-Web-Server.tar.gz:
	$(CURL) "https://gitlab.com/eidheim/Simple-Web-Server/-/archive/$(SWS_VER)/Simple-Web-Server-$(SWS_VER).tar.gz" > Simple-Web-Server.tar.gz

Simple-Web-Server: Simple-Web-Server.tar.gz
	$(TAR) -xvzf Simple-Web-Server.tar.gz
	mv Simple-Web-Server-$(SWS_VER) Simple-Web-Server

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
