MAKEFLAGS += -j4

# Compiler flags
CXX      := g++-13
CXXFLAGS := -std=c++20 -pedantic-errors -Wall -Wno-format-security -Wno-sign-compare

# Libraries
LDFLAGS := -L/usr/lib -lbfd -ldw

# Targets
TARGET := main

# Directories
SRC_DIR := src
BIN_DIR := bin

# Common source files
COMMON_SRC := $(SRC_DIR)/Jomini.cpp
COMMON_OBJ := $(BIN_DIR)/Jomini.o

# Build type (default, debug, release)
BUILD_TYPE := debug
ifeq ($(BUILD_TYPE),debug)
    CXXFLAGS += -O0 -DDEBUG -g #-fsanitize=address
else ifeq ($(BUILD_TYPE),release)
    CXXFLAGS += -O3 -DNDEBUG
endif

# Pattern rule for compiling object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -Ivendor -c $< -o $@

# Rules for each executable
$(TARGET): %: $(BIN_DIR)/%.o $(COMMON_OBJ)
# @clear
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$@ $^ $(LDFLAGS)
	@echo "-----------------------------------------"
	@./$(BIN_DIR)/$@

# Build object files for specific executables
$(BIN_DIR)/main.o: $(SRC_DIR)/main.cpp
$(COMMON_OBJ): $(SRC_DIR)/Jomini.cpp $(SRC_DIR)/Jomini.hpp

# Make commands

.PHONY: all clean $(TARGET)
all: build $(BIN_DIR)/$(TARGET)

clean:
	-@rm -rvf $(BIN_DIR)/*