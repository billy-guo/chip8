CC=gcc
CFLAGS=-I. -lGL -lglut
DEPS=mem.h cpu.h
OBJ=mem.c cpu.c

chip8: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) `sdl2-config --cflags --libs`
