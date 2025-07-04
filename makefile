# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++20 -fPIC -Wall -Wextra -Icommon
LDFLAGS := -shared

# Source files
SRC_DIR := Algorithm/source
BUILD_DIR := Algorithm/
PLAYER_SRC := $(SRC_DIR)/Player_212535058_324022904.cpp
TANK_SRC := $(SRC_DIR)/TankAlgorithm_212535058_324022904.cpp

# Target
TARGET := $(BUILD_DIR)/Algorithm.so

# Ensure the build directory exists
$(shell mkdir -p $(BUILD_DIR))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(PLAYER_SRC) $(TANK_SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)
