# Compiler
CC = gcc

# Compiler flags-fsanitize=address 
CFLAGS =   -fopenmp -Wall -Wextra -g -I./include

# Linker flags
LDFLAGS = -lSDL2 -lGL -lGLEW -lm -fopenmp 

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Executable name
EXEC = $(BIN_DIR)/nbody_sim

# Default target
all: $(EXEC)

# Rule to create object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to create the executable
$(EXEC): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

# Clean rule
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) *.txt

# Phony targets
.PHONY: all clean

