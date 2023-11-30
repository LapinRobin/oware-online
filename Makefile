# Compiler and compile options
CC=gcc
CFLAGS=-Wall -g
LIBS=-lm

# Directories
CLIENT_DIR=Client
SERVER_DIR=Serveur

# Executable names (placed inside their respective directories)
CLIENT_EXE=$(CLIENT_DIR)/client_app
SERVER_EXE=$(SERVER_DIR)/server_app

# Source files
CLIENT_SRC=$(CLIENT_DIR)/client2.c
SERVER_SRC=$(SERVER_DIR)/server2.c

# Object files
CLIENT_OBJ=$(CLIENT_SRC:.c=.o)
SERVER_OBJ=$(SERVER_SRC:.c=.o)

# Default target
all: $(CLIENT_EXE) $(SERVER_EXE)

# Client program
$(CLIENT_EXE): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Server program
$(SERVER_EXE): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# To remove generated files
clean:
	rm -f $(CLIENT_DIR)/*.o $(SERVER_DIR)/*.o $(CLIENT_EXE) $(SERVER_EXE)

# Phony targets
.PHONY: all clean
