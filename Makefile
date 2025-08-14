# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -pedantic -g -O0
LDFLAGS := -ldl -rdynamic
TARGET := main

# Source files (adjust paths as needed)
SRCS := \
    main.cpp \
    Simulator/source/Simulator.cpp \
    Simulator/source/AlgorithmRegistrar.cpp \
    Simulator/source/GameManagerRegistrar.cpp \
    Simulator/source/GameManagerRegistration.cpp \
    Simulator/source/PlayerRegistration.cpp \
    Simulator/source/TankAlgorithmRegistration.cpp \
    Simulator/source/MapReader.cpp

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
