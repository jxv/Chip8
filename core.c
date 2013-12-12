/* core.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "core.h"

#define	X(op) ((op >> 8) & 0xf)
#define	Y(op) ((op >> 4) & 0xf)
#define	N(op) (op & 0xf)
#define	K(op) (op & 0xf)
#define	NN(op) (op & 0xff)
#define	KK(op) (op & 0xff)
#define	NNN(op) (op & 0xfff)
#define	KKK(op) (op & 0xfff)

void chip8_time_step(struct Chip8 *c)
{
	if (c->delay_timer > 0) {
		c->delay_timer--;
	}
	if (c->sound_timer > 0) {
		c->sound_timer--;
	}
}

void opcode_00E0(struct Chip8 *c, u16 op)
{
	memset(c->display, 0, sizeof(0[c->display]) * 32 * 64);
}

void opcode_00EE(struct Chip8 *c, u16 op)
{
	c->stack_pointer--;
	c->program_counter = c->stack[c->stack_pointer];
}

void opcode_0___(struct Chip8 *c, u16 op)
{
	switch (op & 0xff) {
	case 0xe0: opcode_00E0(c, op); break;
	case 0xee: opcode_00EE(c, op); break;
	default: break;	/* no rca */
	}
}

void opcode_1NNN(struct Chip8 *c, u16 op)
{
	c->program_counter = NNN(op);
	c->program_counter -= 2;
}

void opcode_2NNN(struct Chip8 *c, u16 op)
{
	c->stack[c->stack_pointer] = c->program_counter;
	c->stack_pointer++;
	c->program_counter = NNN(op);
}

void opcode_3XNN(struct Chip8 *c, u16 op)
{
	if (c->v[X(op)] == KK(op)) {
		c->program_counter += 2;
	}
}

void opcode_4XNN(struct Chip8 *c, u16 op)
{
	if (c->v[X(op)] != KK(op)) {
		c->program_counter += 2;
	}
}

void opcode_5XY0(struct Chip8 *c, u16 op)
{
	if (c->v[X(op)] == c->v[Y(op)]) {
		c->program_counter += 2;
	}
}

void opcode_6XNN(struct Chip8 *c, u16 op)
{
	c->v[X(op)] = NN(op);
}

void opcode_7XNN(struct Chip8 *c, u16 op)
{
	c->v[X(op)] += NN(op);
}

void opcode_8XY4(struct Chip8 *c, u16 op)
{
	u16 vx, vy, res;

	vx = c->v[X(op)];
	vy = c->v[Y(op)];
	res = vx + vy;
	c->v[0xf] = res > 255;
	c->v[X(op)] = res & 0xff;
}

void opcode_8XY5(struct Chip8 *c, u16 op)
{
	u16 vx, vy, res;

	vx = c->v[X(op)];
	vy = c->v[Y(op)];
	res = vx - vy;
	c->v[0xf] = vx >= vy;
	c->v[X(op)] = res;
}

void opcode_8XY6(struct Chip8 *c, u16 op)
{
	c->v[0xf] = c->v[Y(op)] & 0x1;
	c->v[X(op)] = c->v[Y(op)] >> 1;
}

void opcode_8XY7(struct Chip8 *c, u16 op)
{
	u16 vx, vy, res;

	vx = c->v[X(op)];
	vy = c->v[Y(op)];
	res = vy - vx;
	c->v[0xf] = vy >= vx;
	c->v[X(op)] = res;
}

void opcode_8XYE(struct Chip8 *c, u16 op)
{
	c->v[0xf] = c->v[Y(op)] >> 7;
	c->v[X(op)] = c->v[Y(op)] << 1;
}

void opcode_8XY_(struct Chip8 *c, u16 op)
{
	switch (K(op)) {
	case 0x0: c->v[X(op)] = c->v[Y(op)]; break;
	case 0x1: c->v[X(op)] |= c->v[Y(op)]; break;
	case 0x2: c->v[X(op)] &= c->v[Y(op)]; break;
	case 0x3: c->v[X(op)] ^= c->v[Y(op)]; break;
	case 0x4: opcode_8XY4(c, op); break;
	case 0x5: opcode_8XY5(c, op); break;
	case 0x6: opcode_8XY6(c, op); break;
	case 0x7: opcode_8XY7(c, op); break;
	case 0xe: opcode_8XYE(c, op); break;
	default: break;
	}
}

void opcode_9XY0(struct Chip8 *c, u16 op)
{
	if (c->v[X(op)] != c->v[Y(op)]) {
		c->program_counter += 2;
	}
}

void opcode_ANNN(struct Chip8 *c, u16 op)
{
	c->indirect_addr = KKK(op);
}

void opcode_BNNN(struct Chip8 *c, u16 op)
{
	c->program_counter = NNN(op) + c->v[0x0];
	c->program_counter -= 2;
}

void opcode_CXNN(struct Chip8 *c, u16 op)
{
	c->v[X(op)] = ((u8) (rand() % 256)) & KK(op);
}

void opcode_DXYN(struct Chip8 *c, u16 op)
{
	u16 x, y, n, p, u, w, d, a, b;

	x = X(op);
	y = Y(op);
	n = N(op);

	c->v[0xf] = 0x0;
	for (w = 0; w < n; w++) {
		b = (w + c->v[y]) % 32;
		p = c->memory[c->indirect_addr + w];
		for (u = 0; u < 8; u++) {
			a = (u + c->v[x]) % 64;
			d = ! !(p & (0x80 >> u));
			c->display[b * 64 + a] ^= d;
			c->v[0xf] |= d;
		}
	}
}

void opcode_EX9E(struct Chip8 *c, u16 op)
{
	if (c->key[c->v[X(op)]]) {
		c->program_counter += 2;
	}
}

void opcode_EXA1(struct Chip8 *c, u16 op)
{
	if (!c->key[c->v[X(op)]]) {
		c->program_counter += 2;
	}
}

void opcode_EX__(struct Chip8 *c, u16 op)
{
	switch (KK(op)) {
	case 0x9e: opcode_EX9E(c, op); break;
	case 0xa1: opcode_EXA1(c, op); break;
	default: break;
	}
}

void opcode_FX1E(struct Chip8 *c, u16 op)
{
	c->indirect_addr += c->v[X(op)];
	c->v[0xf] = c->indirect_addr > 0xfff;
}

void opcode_FX0A(struct Chip8 *c, u16 op)
{
	u8 i;
	for (i = 0x0; i < 0x10; i++) {
		if (c->key[i]) {
			c->v[X(op)] = i;
			return;
		}
	}
	c->program_counter -= 2;
}

void opcode_FX29(struct Chip8 *c, u16 op)
{
	c->indirect_addr = (c->v[X(op)] & 0xf) * 5;
}

void opcode_FX33(struct Chip8 *c, u16 op)
{
	c->memory[c->indirect_addr] = c->v[X(op)] / 100;
	c->memory[c->indirect_addr + 1] = c->v[X(op)] % 100 / 10;
	c->memory[c->indirect_addr + 2] = c->v[X(op)] % 10;
}

void opcode_FX55(struct Chip8 *c, u16 op)
{
	memcpy(c->memory + c->indirect_addr, c->v, X(op) + 1);
	c->indirect_addr += X(op) + 1;
}

void opcode_FX65(struct Chip8 *c, u16 op)
{
	memcpy(c->v, c->memory + c->indirect_addr, X(op) + 1);
	c->indirect_addr += X(op) + 1;
}

void opcode_FX__(struct Chip8 *c, u16 op)
{
	switch (KK(op)) {
	case 0x07: c->v[X(op)] = c->delay_timer;	break;
	case 0x0a: opcode_FX0A(c, op); break;
	case 0x15: c->delay_timer = c->v[X(op)]; break;
	case 0x18: c->sound_timer = c->v[X(op)];	break;
	case 0x1e: opcode_FX1E(c, op); break;
	case 0x29: opcode_FX29(c, op); break;
	case 0x33: opcode_FX33(c, op); break;
	case 0x55: opcode_FX55(c, op); break;
	case 0x65: opcode_FX65(c, op); break;
	default: break;
	}
}

void load_font_0(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0x90;
	mem[3] = 0x90;
	mem[4] = 0xF0;
}

void load_font_1(u8 * mem)
{
	mem[0] = 0x20;
	mem[1] = 0x60;
	mem[2] = 0x20;
	mem[3] = 0x20;
	mem[4] = 0x70;
}

void load_font_2(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x10;
	mem[2] = 0xF0;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

void load_font_3(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x10;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0xF0;
}

void load_font_4(u8 * mem)
{
	mem[0] = 0x90;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0x10;
}

void load_font_5(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0xF0;
}

void load_font_6(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x90;
	mem[4] = 0xF0;
}

void load_font_7(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x10;
	mem[2] = 0x20;
	mem[3] = 0x40;
	mem[4] = 0x40;
}

void load_font_8(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x90;
	mem[4] = 0xF0;
}

void load_font_9(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0xF0;
}

void load_font_A(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x90;
	mem[4] = 0x90;
}

void load_font_B(u8 * mem)
{
	mem[0] = 0xE0;
	mem[1] = 0x90;
	mem[2] = 0xE0;
	mem[3] = 0x90;
	mem[4] = 0xE0;
}

void load_font_C(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0x80;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

void load_font_D(u8 * mem)
{
	mem[0] = 0xE0;
	mem[1] = 0x90;
	mem[2] = 0x90;
	mem[3] = 0x90;
	mem[4] = 0xE0;
}

void load_font_E(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

void load_font_F(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x80;
	mem[4] = 0x80;
}

void load_system_font(struct Chip8 *c)
{
	load_font_0(c->memory + 0);
	load_font_1(c->memory + 5);
	load_font_2(c->memory + 10);
	load_font_3(c->memory + 15);
	load_font_4(c->memory + 20);
	load_font_5(c->memory + 25);
	load_font_6(c->memory + 30);
	load_font_7(c->memory + 35);
	load_font_8(c->memory + 40);
	load_font_9(c->memory + 45);
	load_font_A(c->memory + 50);
	load_font_B(c->memory + 55);
	load_font_C(c->memory + 60);
	load_font_D(c->memory + 65);
	load_font_E(c->memory + 70);
	load_font_F(c->memory + 75);
}

void chip8_load_system(struct Chip8 *c)
{
	memset(c->v, 0, sizeof(0[c->v]) * 0x10);
	memset(c->key, 0, sizeof(0[c->key]) * 0x10);
	memset(c->stack, 0, sizeof(0[c->stack]) * 0x10);
	memset(c->memory, 0, sizeof(0[c->memory]) * 0x1000);
	memset(c->display, false, sizeof(0[c->display]) * 32 * 64);

	load_system_font(c);
	c->stack_pointer = 0;
	c->program_counter = 0x200;
	c->indirect_addr = 0;
	c->sound_timer = 0;
	c->delay_timer = 0;
}

void chip8_step(struct Chip8 *c)
{
	u16 op;

	op = c->memory[c->program_counter] << 8 | c->memory[c->program_counter + 1];

	switch ((op & 0xf000) >> 12) {
	case 0x0: opcode_0___(c, op); break;
	case 0x1: opcode_1NNN(c, op); break;
	case 0x2: opcode_2NNN(c, op); break;
	case 0x3: opcode_3XNN(c, op); break;
	case 0x4: opcode_4XNN(c, op); break;
	case 0x5: opcode_5XY0(c, op); break;
	case 0x6: opcode_6XNN(c, op); break;
	case 0x7: opcode_7XNN(c, op); break;
	case 0x8: opcode_8XY_(c, op); break;
	case 0x9: opcode_9XY0(c, op); break;
	case 0xa: opcode_ANNN(c, op); break;
	case 0xb: opcode_BNNN(c, op); break;
	case 0xc: opcode_CXNN(c, op); break;
	case 0xd: opcode_DXYN(c, op); break;
	case 0xe: opcode_EX__(c, op); break;
	case 0xf: opcode_FX__(c, op); break;
	default: break;
	}

	c->program_counter += 2;

}

