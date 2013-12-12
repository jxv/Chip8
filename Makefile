LIBS = `sdl-config --libs`
CFLAGS = -Wall -pedantic -Werror `sdl-config --cflags` -std=c11
CC = gcc
OUT = chip8
SRC = core.c main.c
RM = rm

all: 
	${CC} ${CFLAGS} -o ${OUT} ${SRC} ${LIBS}

clean:
	$(RM) $(OUT)
