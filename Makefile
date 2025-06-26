INCLUDE_DIR=./include
SOURCE_DIR=./src
BIN_DIR=./bin

CXX:= g++
CXXFLAGS:= -I$(INCLUDE_DIR) -std=c++17 -Wall -Wextra -MMD
SDL_FLAGS:= $(shell sdl2-config --cflags --libs)

SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp, $(BIN_DIR)/%.o, $(SOURCES))
DEPS    := $(OBJECTS:.o=.d)
BINARIES := $(BIN_DIR)/cv $(BIN_DIR)/vco $(BIN_DIR)/scope

all: $(BINARIES)

$(BIN_DIR)/cv: $(SOURCE_DIR)/cv.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR)/vco: $(SOURCE_DIR)/vco.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR)/scope: $(SOURCE_DIR)/scope.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(SDL_FLAGS) -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -f $(BIN_DIR)/*

test: all
	$(BIN_DIR)/cv --duration 1 | $(BIN_DIR)/vco --sensitivity 1 | $(BIN_DIR)/scope --horizontal_scale 48000 --trigger_offset 0 --trigger_threshold 0.500 --trigger

# Include dependency files
-include $(DEPS)

.PHONY: all clean test
