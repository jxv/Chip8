/* core.h */

#ifndef CORE_H
#define CORE_H

typedef unsigned char u8;
typedef unsigned short int u16;

struct chip8 {
	u8 v[0x10];
	u8 mem[0x1000];
	u16 stk[0xc];
	u8 sp;
	u16 pc;
	u16 i;
	u16 key[0x10];
	u8 st;
	u8 dt;
	u8 disp[32 * 64];
};

void ldSysData(struct chip8 *);
void timestep(struct chip8 *);
void step(struct chip8 *);

#endif
