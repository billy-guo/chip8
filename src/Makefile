CC=gcc
CFLAGS=-I. -lGL -lglut
DEPS=mem.h cpu.h instructions.h
OBJ=mem.c cpu.c instructions.c

chip8: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) `sdl2-config --cflags --libs`
