#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "chip8.h"

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

static void mk_window(SDL_Surface **s)
{
	SDL_Init(SDL_INIT_VIDEO);
	*s = SDL_SetVideoMode(64, 32, 8, SDL_SWSURFACE);
	SDL_WM_SetCaption("chip8", NULL);
}

static void keys(chip8_t *c, const u8 *ks)
{
	c->key[0x0] = !!ks[SDLK_COMMA];
	c->key[0x1] = !!ks[SDLK_7];
	c->key[0x2] = !!ks[SDLK_8];
	c->key[0x3] = !!ks[SDLK_9];
	c->key[0x4] = !!ks[SDLK_u];
	c->key[0x5] = !!ks[SDLK_i];
	c->key[0x6] = !!ks[SDLK_o];
	c->key[0x7] = !!ks[SDLK_j];
	c->key[0x8] = !!ks[SDLK_k];
	c->key[0x9] = !!ks[SDLK_l];
	c->key[0xa] = !!ks[SDLK_m];
	c->key[0xb] = !!ks[SDLK_PERIOD];
	c->key[0xc] = !!ks[SDLK_0];
	c->key[0xd] = !!ks[SDLK_p];
	c->key[0xe] = !!ks[SDLK_SEMICOLON];
	c->key[0xf] = !!ks[SDLK_SLASH];
}

static void draw(const chip8_t *c, SDL_Surface *s)
{
	const u8 sound_color = 0xa0;
	const u8 bg_color = 0x00;
	const u8 fg_color = 0xff;
	const u8 color = c->sound_timer ? sound_color : fg_color;
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 64; x++) {
			u8 *p = (u8 *)s->pixels +
				x * s->format->BytesPerPixel +
				y * s->pitch;
			const bool q = c->display[y * 64 + x];
			*p = q ? color : bg_color;
		}
	}
}

static void run(chip8_t *c, SDL_Surface *s)
{
	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event))
			if (event.type == SDL_QUIT)
				quit = true;
		const u8 *ks = SDL_GetKeyState(NULL);
		keys(c, ks);
		chip8_time_step(c);
		chip8_step(c);
		draw(c, s);
		SDL_Delay(20);	/* aprx. 60Hz */
		SDL_Flip(s);
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
	SDL_Surface *s;
	mk_window(&s);
	run(&c, s);
	return EXIT_SUCCESS;
}
