# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++20 -fPIC -Wall -Wextra -I./header -I../common -I../UserCommon/source -fvisibility=hidden
LDFLAGS := -shared -ldl -Wl,--export-dynamic

# Directories
SRC_DIR := ./source
TARGET := GameManager_212535058_324022904.so

# Source files - using your exact paths
GAMEMANAGER_CPP := $(SRC_DIR)/GameManager.cpp \
                   $(SRC_DIR)/BoardManager.cpp
USER_COMMON_CPP := $(wildcard ../UserCommon/source/*.cpp)

# Object files
OBJECTS := $(GAMEMANAGER_CPP:.cpp=.o) $(USER_COMMON_CPP:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "Linking $@"
	$(CXX) $(LDFLAGS) $^ -o $@

# Rule for GameManager sources
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule for UserCommon sources
../UserCommon/source/%.o: ../UserCommon/source/%.cpp
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleaned build files"