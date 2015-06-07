CC = g++
GXX = g++

EXE = backup
OBJ = backup.o md5.o

FLAGS = -O2 -g -Wall
CFLAGS = $(FLAGS)
CXXFLAGS = $(FLAGS) -std=c++11

LDLIBS = -lboost_system -lboost_filesystem

.PHONY: default
default: $(EXE)

$(EXE): $(OBJ)

$(OBJ): backup.cpp backup.hpp md5.cpp md5.hpp

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXE)

.PHONY: all
all: clean default
