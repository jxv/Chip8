#include <SDL/SDL.h>
#include "backend.h"

SDL_Surface *screen = NULL;

void mk_window()
{
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(64, 32, 8, SDL_SWSURFACE);
	SDL_WM_SetCaption("chip8", NULL);
}


bool keys(chip8_t *c)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
		if (event.type == SDL_QUIT)
			return true;
	const u8 *ks = SDL_GetKeyState(NULL);
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
	return false;
}

void draw(const chip8_t *c)
{
	const u8 sound_color = 0xa0;
	const u8 bg_color = 0x00;
	const u8 fg_color = 0xff;
	const u8 color = c->sound_timer ? sound_color : fg_color;
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 64; x++) {
			u8 *p = (u8 *)screen->pixels +
				x * screen->format->BytesPerPixel +
				y * screen->pitch;
			const bool q = c->display[y * 64 + x];
			*p = q ? color : bg_color;
		}
	}
}

void sync()
{
	SDL_Delay(20);	/* aprx. 60Hz */
	SDL_Flip(screen);
}
