# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++20 -fPIC -Wall -Wextra \
            -I./header -I../common -I../UserCommon/source \
            -fvisibility=hidden
LDFLAGS := -shared -ldl -Wl,--export-dynamic

# Directories
SRC_DIR := ./source
TARGET := GameManager_212535058_324022904.so

# Source files
GAMEMANAGER_CPP := $(SRC_DIR)/GameManager.cpp \
                   $(SRC_DIR)/BoardManager.cpp
USER_COMMON_CPP := $(wildcard ../UserCommon/source/*.cpp)

# Build target
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(GAMEMANAGER_CPP) $(USER_COMMON_CPP)
	@echo "Compiling and linking $@"
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)
	@echo "Cleaned build files"
