CC = gcc
GXX = g++

EXE = backup
OBJ = backup.o md5.o

FLAGS = -O2
CFLAGS = $(FLAGS)
CXXFLAGS = $(FLAGS) -std=c++11

.PHONY: default
default: $(EXE)

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXE)

.PHONY: all
all: clean default
