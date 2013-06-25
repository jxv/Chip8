LIBS = `sdl-config --libs`
CFLAGS = -Wall -pedantic -Werror `sdl-config --cflags` -ansi
CC = cc
OUT = Chip8Emulator
SRC = core.c main.c
RM = rm

all: 
	${CC} ${CFLAGS} -o ${OUT} ${SRC} ${LIBS}

clean:
	$(RM) $(OUT)
