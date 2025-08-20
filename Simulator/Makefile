# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -pedantic 
LDFLAGS := -ldl -rdynamic 
TARGET := ../simulator_212535058_324022904

# Source files (adjust paths as needed)
SRC_DIR := ./source
SRCS := \
    ./main.cpp \
    $(SRC_DIR)/Simulator.cpp \
    $(SRC_DIR)/AlgorithmRegistrar.cpp \
    $(SRC_DIR)/GameManagerRegistrar.cpp \
    $(SRC_DIR)/GameManagerRegistration.cpp \
    $(SRC_DIR)/PlayerRegistration.cpp \
    $(SRC_DIR)/TankAlgorithmRegistration.cpp \
    $(SRC_DIR)/MapReader.cpp

# Build rules
all: $(TARGET)

$(TARGET): $(SRCS)
	@echo "Compiling and linking $@"
	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $@

clean:
	rm -f $(TARGET)
	@echo "Cleaned build files"

# Special rule to verify debug symbols
check-symbols: $(TARGET)
	@echo "Checking debug symbols..."
	@readelf --debug-dump $(TARGET) | head -20 || objdump --syms $(TARGET) | grep debug

.PHONY: all clean check-symbols
