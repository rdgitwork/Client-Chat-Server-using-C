# Define compiler and flags
CC=gcc
CFLAGS=-pthread -lncurses

# Define paths
SRC_DIR=./src
INC_DIR=./inc
OBJ_DIR=./obj
BIN_DIR=./bin

# Define target executables
TARGETS=$(BIN_DIR)/chat-client $(BIN_DIR)/chat-server

# Default target
all: $(TARGETS)

# Chat client
$(BIN_DIR)/chat-client: $(OBJ_DIR)/chat_client.o
	$(CC) $^ -o $@ $(CFLAGS)

# Chat server
$(BIN_DIR)/chat-server: $(OBJ_DIR)/chat_server.o
	$(CC) $^ -o $@ $(CFLAGS)

# Generic rule for compiling object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(INC_DIR)

# Clean target
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGETS)

.PHONY: all clean

