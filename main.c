/* main.c */

#include <SDL/SDL.h>
#include <stdio.h>
#include "core.h"

int fdArgErr(int, char **);
int rdRom(struct chip8 *, char *);
void mkWindow(SDL_Surface **);
void exec(struct chip8 *, SDL_Surface *);
void keys(struct chip8 *c, Uint8 * ks);
void draw(struct chip8 *c, SDL_Surface *);

int main(int argc, char *argv[])
{
	SDL_Surface *s;
	int err;
	struct chip8 c;

	err = fdArgErr(argc, argv);
	if (err != 0) {
		return err;
	}
	ldSysData(&c);
	err = rdRom(&c, argv[1]);
	if (err != 0) {
		return err;
	}
	mkWindow(&s);
	exec(&c, s);
	return 0;
}

int fdArgErr(int c, char *v[])
{
	if (c != 2) {
		return 1;	/* bad args */
	}
	return 0;
}

int rdRom(struct chip8 *c, char *p)
{
	FILE *fp;
	int x, i;

	fp = fopen(p, "r");
	if (!fp) {
		return 2;	/* no rom */
	}
	i = 0x200;
	while (i < 0xFFF && ((x = fgetc(fp)) != EOF)) {
		c->mem[i] = x;
		i++;
	}
	fclose(fp);
	if (i >= 0xFFF) {
		return 3;	/* too large */
	}
	return 0;
}

void mkWindow(SDL_Surface ** s)
{
	SDL_Init(SDL_INIT_VIDEO);
	*s = SDL_SetVideoMode(64, 32, 8, SDL_SWSURFACE);
	SDL_WM_SetCaption("EmuChip8", "EmuChip8");
}

void exec(struct chip8 *c, SDL_Surface * s)
{
	SDL_Event e;
	int q = 0;
	Uint8 *ks;
	ks = SDL_GetKeyState(NULL);
	while (!q) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				q = 1;
			}
		}
		ks = SDL_GetKeyState(NULL);
		keys(c, ks);
		timestep(c);
		step(c);
		draw(c, s);
		SDL_Delay(20);	/* aprx. 60Hz */
		SDL_Flip(s);
	}
}

void keys(struct chip8 *c, Uint8 * ks)
{
	c->key[0x0] = ! !ks[SDLK_COMMA];
	c->key[0x1] = ! !ks[SDLK_7];
	c->key[0x2] = ! !ks[SDLK_8];
	c->key[0x3] = ! !ks[SDLK_9];
	c->key[0x4] = ! !ks[SDLK_u];
	c->key[0x5] = ! !ks[SDLK_i];
	c->key[0x6] = ! !ks[SDLK_o];
	c->key[0x7] = ! !ks[SDLK_j];
	c->key[0x8] = ! !ks[SDLK_k];
	c->key[0x9] = ! !ks[SDLK_l];
	c->key[0xa] = ! !ks[SDLK_m];
	c->key[0xb] = ! !ks[SDLK_PERIOD];
	c->key[0xc] = ! !ks[SDLK_0];
	c->key[0xd] = ! !ks[SDLK_p];
	c->key[0xe] = ! !ks[SDLK_SEMICOLON];
	c->key[0xf] = ! !ks[SDLK_SLASH];
}

void draw(struct chip8 *c, SDL_Surface * s)
{
	int x, y, q;
	Uint8 *p;
	Uint8 st, bg, fg, cl;

	st = 0xa0;
	bg = 0x01;
	fg = 0x0b;
	cl = c->st ? st : fg;
	for (y = 0; y < 32; y++) {
		for (x = 0; x < 64; x++) {
			p = (Uint8 *) s->pixels + x * s->format->BytesPerPixel +
			    y * s->pitch;
			q = c->disp[y * 64 + x];
			*p = q ? cl : bg;
		}
	}
}
