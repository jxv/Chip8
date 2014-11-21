LIB := -lc -lm
CFLAGS := -Wall -pedantic -Werror -std=c99 -DNDEBUG
OUT := chip8
SRC := chip8.c main.c
RM := rm

sdl:
	$(CC) -o $(OUT) $(CFLAGS) $(shell sdl-config --cflags) $(SRC) sdl.c $(LIB) $(shell sdl-config --libs)

xcb:
	$(CC) -o $(OUT) $(CFLAGS) $(shell pkg-config --cflags xcb) $(SRC) xcb.c $(LIB) $(shell pkg-config --libs xcb)

all:	sdl

clean:
	$(RM) $(OUT)
