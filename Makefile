CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Source files
SRCS = src/main.c src/filesys.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
EXEC = filesys

# Dependency files
DEPS = src/filesys.h

# Default target
all: $(EXEC)

# Build executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean target
clean:
	rm -f $(EXEC) $(OBJS)
