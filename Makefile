# Compiler and flags
CC := mpic++
CFLAGS := -O3 -Wall -Wextra -std=c++11 -fopenmp

# Automatically find all .cpp files in the current directory
SRC := $(wildcard src/*.cpp)
TEST := tests/23 

# Name of the output binary
TARGET := shsup

# Default rule
all: $(TARGET)

# Link all .cpp files into one executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	srun --label --mpi=pmix -N 5 ./$(TARGET) $(TEST)


# Cleanup
clean:
	rm -f $(TARGET)

.PHONY: all clean

