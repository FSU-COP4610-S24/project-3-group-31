CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
SRC_DIR = src
INCLUDE_DIR = include
BIN_DIR = bin

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,%.o,$(SRCS))

# Executable name
EXEC = $(BIN_DIR)/filesys

# Dependency files
DEPS = $(wildcard $(INCLUDE_DIR)/*.h)

# Default target
all: $(EXEC)

# Build executable
$(EXEC): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files
%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean target
clean:
	rm -rf $(BIN_DIR) $(OBJS)
