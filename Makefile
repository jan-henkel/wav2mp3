EXE = wav2mp3

SRC_DIR = src
ifeq ($(OS),Windows_NT)
	LIB_DIR = lib/windows
else
	LIB_DIR = lib/linux
endif

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=%.o)
STATIC_LIBS = $(wildcard $(LIB_DIR)/*.a)

CPPFLAGS += -Iinclude -std=c++11
CFLAGS += -Wall
LDLIBS += -lpthread

CC = g++

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(STATIC_LIBS) $(LDLIBS) -o $@

%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)
