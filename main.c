#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "chip8.h"
#include "backend.h"

static int find_arg_err(int c, const char *v[])
{
	if (c != 2)
		return 1;	/* bad args */
	return 0;
}

static int read_rom(chip8_t *c, const char *p)
{
	FILE *fp = fopen(p, "r");
	if (!fp)
		return 2; /* no rom */
	for (int i = 0x200; i < 0xfff; i++) {
		int x = fgetc(fp);
		if (x == EOF)
			break;
		c->mem[i] = x;
	}
	fclose(fp);
	return 0;
}

static void run(chip8_t *c)
{
	bool quit = false;
	while (!quit) {
		quit = keys(c);
		chip8_time_step(c);
		chip8_step(c);
		draw(c);
		sync();
	}
}

int main(int argc, const char *argv[])
{
	int err = find_arg_err(argc, argv);
	if (err != 0)
		return err;
	chip8_t c;
	chip8_load_system(&c);
	err = read_rom(&c, argv[1]);
	if (err != 0)
		return err;
	open_window();
	run(&c);
	close_window();
	return EXIT_SUCCESS;
}
