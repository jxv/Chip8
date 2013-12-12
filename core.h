/* core.h */

#ifndef CORE_H
#define CORE_H

#include <stdbool.h>

typedef unsigned char u8;
typedef unsigned short int u16;

struct Chip8 {
	u8  v[0x10];
	u8  memory[0x1000];
	u16 stack[0xc];
	u8  stack_pointer;
	u16 program_counter;
	u16 indirect_addr;
	u16 key[0x10];
	u8  sound_timer;
	u8  delay_timer;
	bool display[32 * 64];
};

void chip8_load_system(struct Chip8*);
void chip8_time_step(struct Chip8 *);
void chip8_step(struct Chip8 *);

#endif
