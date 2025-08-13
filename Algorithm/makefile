# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++20 -fPIC -Wall -Wextra -Icommon -fvisibility=hidden
LDFLAGS := -shared -ldl -Wl,--export-dynamic

# Source files
SRC_DIR := ./source
BUILD_DIR := .
USER_COMMON_DIR := ../UserCommon/source
PLAYER_SRC := $(SRC_DIR)/Player_212535058_324022904.cpp
TANK_SRC := $(SRC_DIR)/TankAlgorithm_212535058_324022904.cpp $(SRC_DIR)/MyTankAlgorithm.cpp $(SRC_DIR)/TankBattleInfo.cpp
USER_COMMON_SRC := $(USER_COMMON_DIR)/Shell.cpp $(USER_COMMON_DIR)/Wall.cpp $(USER_COMMON_DIR)/Mine.cpp $(USER_COMMON_DIR)/Tank.cpp
# COMMON_DIR := common
# COMMON_SRC := $(COMMON_DIR)/PlayerRegistration.cpp $(COMMON_DIR)/TankAlgorithmRegistration.cpp $(COMMON_SRC)

# Target
TARGET := $(BUILD_DIR)/Algorithm_212535058_324022904.so

# Ensure the build directory exists
$(shell mkdir -p $(BUILD_DIR))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(PLAYER_SRC) $(TANK_SRC) $(USER_COMMON_SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(TARGET)
