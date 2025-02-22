# Variables
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -w
LDFLAGS = -lz -lcrypto
MATH = -lm
TARGET = git_clone
SRC_FILES = hash_table.c dynamic_array.c ini_parser.c repository.c utils.c bridges.c git_object.c kvlm.c tree_parser.c refs.c git_index.c git_ignore.c main.c
GDB = -g

# Build target
all: $(TARGET)

# Compile the source files (main.c, exercise.c, munit.c) into the TARGET executable
$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_FILES) $(LDFLAGS) $(MATH) $(GDB)

# Clean target (removes the executable)
clean:
	rm -f $(TARGET)

# Run the tests
run-tests: $(TARGET)
	./$(TARGET)

# PHONY ensures that these targets will run even if a file with the same name exists
.PHONY: all clean run-tests
