INCLUDE_DIR = ./include
SOURCE_DIR  = ./src
BIN_DIR     = ./bin

CXX        := g++
CXXFLAGS   := -I$(INCLUDE_DIR) -std=c++20 -Wall -Wextra -MMD
SDL_FLAGS  := $(shell sdl2-config --cflags --libs)

# Source and object files
SOURCES    := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS    := $(patsubst $(SOURCE_DIR)/%.cpp, $(BIN_DIR)/%.o, $(SOURCES))
DEPS       := $(OBJECTS:.o=.d)

# Binaries
BINARIES   := $(BIN_DIR)/cv $(BIN_DIR)/vco $(BIN_DIR)/scope $(BIN_DIR)/filter

all: $(BINARIES)

# Pattern rule for object files
$(BIN_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Individual targets
$(BIN_DIR)/cv: $(BIN_DIR)/cv.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BIN_DIR)/vco: $(BIN_DIR)/vco.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BIN_DIR)/filter: $(BIN_DIR)/filter.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BIN_DIR)/scope: $(BIN_DIR)/scope.o
	$(CXX) $(CXXFLAGS) $^ $(SDL_FLAGS) -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -f $(BIN_DIR)/*

test: all
	$(BIN_DIR)/cv --duration 3 | $(BIN_DIR)/vco --wave_type square --sensitivity 100 | $(BIN_DIR)/filter --filter_type lowpass --cutoff 3000 --rolloff 12 --sample_rate 48000 | $(BIN_DIR)/filter --filter_type highpass --cutoff 1000 --rolloff 12 --sample_rate 48000 | $(BIN_DIR)/scope --sample_rate 48000 --trigger --trigger_offset 100 --trigger_threshold 0.5 --time_divisions 20 --time_per_division .001 --voltage_divisions 10 --voltage_per_division 0.2

# Include auto-generated dependency files
-include $(DEPS)

.PHONY: all clean test

