# Directories
SRC_DIR := src
BIN_DIR := bin

# Source files and output directory
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
BINARIES := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%, $(SRC_FILES))

# Compiler and compiler flags
CC := gcc
BASE_FLAGS := -Wall -Wextra -pedantic
DEBUG_FLAGS := $(BASE_FLAGS) -g -O0
RELEASE_FLAGS := $(BASE_FLAGS) -O2

# Targets
.PHONY: all debug release clean

# Default target
all: debug

# Rule for the debug build
debug: CFLAGS := $(DEBUG_FLAGS)
debug: $(BINARIES)

# Rule for the release build
release: CFLAGS := $(RELEASE_FLAGS)
release: $(BINARIES)

# Rule to build binaries
$(BIN_DIR)/%: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) $< -o $@
	@echo "  [\033[32m+\033[0m] Compiled \033[36m/$@\033[0m"

# Clean rule
BIN_FILES := $(wildcard $(BIN_DIR)/*)
clean:
	@echo "Cleaning..."

	@$(foreach file,$(BIN_FILES), \
		rm -f $(file); \
		echo "  [\033[32m-\033[0m] Removed \033[36m$(file)\033[0m"; \
	)

	@rm -rf $(BIN_DIR)

	@echo "Done!"
