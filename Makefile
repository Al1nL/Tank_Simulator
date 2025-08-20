# Main Makefile for Tank Battle Simulator Project

# Project identifiers
ID1 := 212535058
ID2 := 324022904
SUBMITTER_IDS := $(ID1)_$(ID2)

# Directories
SIM_DIR := ./Simulator
GM_DIR := ./GameManager
ALG_DIR := ./Algorithm

# Targets (will be built in respective directories)
SIM_TARGET := $(SIM_DIR)/simulator_$(SUBMITTER_IDS)
GM_TARGET := $(GM_DIR)/GameManager_$(SUBMITTER_IDS).so
ALG_TARGET := $(ALG_DIR)/Algorithm_$(SUBMITTER_IDS).so

# Default target
all: $(SIM_TARGET) $(GM_TARGET) $(ALG_TARGET)

# Build Simulator - just invoke its Makefile
$(SIM_TARGET):
	@echo "Building Simulator..."
	$(MAKE) -C $(SIM_DIR)

# Build GameManager - just invoke its Makefile
$(GM_TARGET):
	@echo "Building GameManager..."
	$(MAKE) -C $(GM_DIR)

# Build Algorithm - just invoke its Makefile
$(ALG_TARGET):
	@echo "Building Algorithm..."
	$(MAKE) -C $(ALG_DIR)

# Clean all build artifacts
clean:
	@echo "Cleaning all build artifacts..."
	$(MAKE) -C $(SIM_DIR) clean
	$(MAKE) -C $(GM_DIR) clean
	$(MAKE) -C $(ALG_DIR) clean

.PHONY: all clean test