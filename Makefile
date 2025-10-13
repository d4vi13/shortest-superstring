# Compiler and flags
CC := g++
CFLAGS := -O2 -Wall -Wextra -std=c++11

# Automatically find all .cpp files in the current directory
SRC := $(wildcard src/*.cpp)
TEST := tests/test 

# Name of the output binary
TARGET := shsup

# Default rule
all: $(TARGET)

# Link all .cpp files into one executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) $(TEST)


# Cleanup
clean:
	rm -f $(TARGET)

.PHONY: all clean

