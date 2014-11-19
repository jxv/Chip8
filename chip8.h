#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;

struct chip8 {
	u8 v[0x10];
	u8 mem[0x1000];
	u16 stack[0xc];
	u8 sp;
	u16 pc;
	u16 idr_addr;
	u16 key[0x10];
	u8  sound_timer;
	u8  delay_timer;
	bool display[32 * 64];
};

typedef struct chip8 chip8_t;

void chip8_load_system(chip8_t *);
void chip8_time_step(chip8_t *);
void chip8_step(chip8_t *);

#endif
