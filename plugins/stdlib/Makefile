PLUGIN_NAME := $(lastword $(subst /, ,$(shell pwd)))
BASE_DIR := ../..
BUILD_DIR := $(BASE_DIR)/build/$(PLUGIN_NAME)

PLUGIN_CPP_FILES := $(wildcard *.cpp)
PLUGIN_D_FILES := $(patsubst %.cpp,$(BUILD_DIR)/%.d,$(PLUGIN_CPP_FILES))
PLUGIN_O_FILES := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(PLUGIN_CPP_FILES))
PLUGIN_ICOLIB := $(BASE_DIR)/$(PLUGIN_NAME).icolib

PLUGIN_CXXFLAGS := -I. $(patsubst -I%,-I$(BASE_DIR)/%,$(CXXFLAGS))

$(PLUGIN_ICOLIB): $(PLUGIN_O_FILES)
	$(CXX) $(PLUGIN_FLAGS) $(PLUGIN_CXXFLAGS) -o $(PLUGIN_ICOLIB) $(PLUGIN_O_FILES) $(LINKFLAGS)

$(PLUGIN_D_FILES): | $(BUILD_DIR)
$(PLUGIN_D_FILES): $(BUILD_DIR)/%.d: %.cpp
	$(CXX) -MM -MT '$(patsubst %.d,%.o,$@) $@' $(PLUGIN_CXXFLAGS) $< > $@
include $(PLUGIN_D_FILES)

$(PLUGIN_O_FILES): | $(BUILD_DIR)
$(PLUGIN_O_FILES): $(BUILD_DIR)/%.o: $(BUILD_DIR)/%.d
	$(CXX) $(PLUGIN_FLAGS) $(PLUGIN_CXXFLAGS) -c -o $@ $(patsubst $(BUILD_DIR)/%.d,%.cpp,$<)

$(BUILD_DIR): | $(BASE_DIR)/build
	mkdir $(BUILD_DIR)
