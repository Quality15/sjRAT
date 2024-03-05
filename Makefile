# Compiler
CC = g++

# Flags
CFLAGS = -lws2_32 -lgdi32 -luser32 -lwinmm -lshlwapi -s -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc -fpermissive -fexceptions

# Curl flags
CURL_FLAGS = -Lcurl/lib -lcurl

# Directories
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = obj

# Source files
CLIENT_SRC = $(wildcard $(SRC_DIR)/client/*.cpp)
SERVER_SRC = $(wildcard $(SRC_DIR)/server/*.cpp)
COMMON_SRC = $(wildcard $(SRC_DIR)/*.cpp)

# Object files
CLIENT_OBJ = $(patsubst $(SRC_DIR)/client/%.cpp,$(OBJ_DIR)/client/%.o,$(CLIENT_SRC))
SERVER_OBJ = $(patsubst $(SRC_DIR)/server/%.cpp,$(OBJ_DIR)/server/%.o,$(SERVER_SRC))
COMMON_OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(COMMON_SRC))

# Targets
TARGETS = client server

# Default target
all: $(TARGETS)

client: $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) -O2 $^ -o $(BUILD_DIR)/$@.exe $(CFLAGS)

server: $(SERVER_OBJ) $(COMMON_OBJ)
	$(CC) -O2 $^ -o $(BUILD_DIR)/$@.exe $(CFLAGS) $(CURL_FLAGS)

# Rule to compile source files into object files
$(OBJ_DIR)/client/%.o: $(SRC_DIR)/client/%.cpp | $(OBJ_DIR)/client
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)/server/%.o: $(SRC_DIR)/server/%.cpp | $(OBJ_DIR)/server
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

# Create directories if they don't exist
$(OBJ_DIR)/client:
	mkdir -p $@

$(OBJ_DIR)/server:
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

# Clean
clean:
	rm -rf $(BUILD_DIR)/* $(OBJ_DIR)/*

.PHONY: all client server clean
