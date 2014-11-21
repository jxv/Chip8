#ifndef __BACKEND__H__
#define __BACKEND__H__

#include "chip8.h"

void mk_window();
bool keys(chip8_t *);
void draw(const chip8_t *);
void sync();

#endif
