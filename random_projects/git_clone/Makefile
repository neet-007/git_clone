# Variables
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -w  # Include the directory where exercise.h is located
LDFLAGS = -lz -lcrypto
TARGET = git_clone
SRC_FILES = hash_table.c dynamic_array.c ini_parser.c repository.c utils.c bridges.c git_object.c kvlm.c main.c

# Build target
all: $(TARGET)

# Compile the source files (main.c, exercise.c, munit.c) into the TARGET executable
$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_FILES) $(LDFLAGS)

# Clean target (removes the executable)
clean:
	rm -f $(TARGET)

# Run the tests
run-tests: $(TARGET)
	./$(TARGET)

# PHONY ensures that these targets will run even if a file with the same name exists
.PHONY: all clean run-tests
