LIBS = `sdl-config --libs`
CFLAGS = -Wall -pedantic -Werror -DDEBUG `sdl-config --cflags` -std=c99
CC = gcc
OUT = chip8
SRC = chip8.c main.c
RM = rm

all: 
	${CC} ${CFLAGS} -o ${OUT} ${SRC} ${LIBS}

clean:
	$(RM) $(OUT)
