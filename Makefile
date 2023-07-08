# compiler
CC = g++

# flags
CFLAGS = -lws2_32 -lgdi32 -luser32 -s -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc -fpermissive -fexceptions

# target name
# TARGET = server.cpp

# output name (.exe)
# OUTPUT = server.exe

# source
SOURCE_DIR = src

# output
OUTPUT_DIR = build

all: server client
	
client:
	$(CC) -O2 $(SOURCE_DIR)/rev-shell.cpp -o $(OUTPUT_DIR)/client.exe $(CFLAGS)

server:
	$(CC) -O2 $(SOURCE_DIR)/server.cpp -o $(OUTPUT_DIR)/server.exe $(CFLAGS)

clean:
	rm $(OUTPUT_DIR)/$(OUTPUT)