#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "chip8.h"

#define	X(op) ((op >> 8) & 0xf)
#define	Y(op) ((op >> 4) & 0xf)
#define	N(op) (op & 0xf)
#define	K(op) (op & 0xf)
#define	NN(op) (op & 0xff)
#define	KK(op) (op & 0xff)
#define	NNN(op) (op & 0xfff)
#define	KKK(op) (op & 0xfff)

void chip8_time_step(chip8_t *c)
{
	if (c->delay_timer > 0)
		c->delay_timer--;
	if (c->sound_timer > 0)
		c->sound_timer--;
}

static void opcode_00E0(chip8_t *c, u16 op)
{
	memset(c->display, 0x00, sizeof(0[c->display]) * 32 * 64);
}

static void opcode_00EE(chip8_t *c, u16 op)
{
	c->sp--;
	c->pc = c->stack[c->sp];
}

static void opcode_0___(chip8_t *c, u16 op)
{
	switch (op & 0xff) {
	case 0xe0:
		opcode_00E0(c, op);
		break;
	case 0xee:
		opcode_00EE(c, op);
		break;
	default:
		break;	/* no rca */
	}
}

static void opcode_1NNN(chip8_t *c, u16 op)
{
	c->pc = NNN(op);
	c->pc -= 2;
}

static void opcode_2NNN(chip8_t *c, u16 op)
{
	c->stack[c->sp] = c->pc;
	c->sp++;
	c->pc = NNN(op);
}

static void opcode_3XNN(chip8_t *c, u16 op)
{
	if (c->v[X(op)] == KK(op))
		c->pc += 2;
}

static void opcode_4XNN(chip8_t *c, u16 op)
{
	if (c->v[X(op)] != KK(op))
		c->pc += 2;
}

static void opcode_5XY0(chip8_t *c, u16 op)
{
	if (c->v[X(op)] == c->v[Y(op)])
		c->pc += 2;
}

static void opcode_6XNN(chip8_t *c, u16 op)
{
	c->v[X(op)] = NN(op);
}

static void opcode_7XNN(chip8_t *c, u16 op)
{
	c->v[X(op)] += NN(op);
}

static void opcode_8XY4(chip8_t *c, u16 op)
{
	const u16 vx = c->v[X(op)];
	const u16 vy = c->v[Y(op)];
	const u16 res = vx + vy;
	c->v[0xf] = res > 255;
	c->v[X(op)] = res & 0xff;
}

static void opcode_8XY5(chip8_t *c, u16 op)
{
	const u16 vx = c->v[X(op)];
	const u16 vy = c->v[Y(op)];
	const u16 res = vx - vy;
	c->v[0xf] = vx >= vy;
	c->v[X(op)] = res;
}

static void opcode_8XY6(chip8_t *c, u16 op)
{
	c->v[0xf] = c->v[Y(op)] & 0x1;
	c->v[X(op)] = c->v[Y(op)] >> 1;
}

static void opcode_8XY7(chip8_t *c, u16 op)
{
	const u16 vx = c->v[X(op)];
	const u16 vy = c->v[Y(op)];
	const u16 res = vy - vx;
	c->v[0xf] = vy >= vx;
	c->v[X(op)] = res;
}

static void opcode_8XYE(chip8_t *c, u16 op)
{
	c->v[0xf] = c->v[Y(op)] >> 7;
	c->v[X(op)] = c->v[Y(op)] << 1;
}

static void opcode_8XY_(chip8_t *c, u16 op)
{
	switch (K(op)) {
	case 0x0:
		c->v[X(op)] = c->v[Y(op)];
		break;
	case 0x1:
		c->v[X(op)] |= c->v[Y(op)];
		break;
	case 0x2:
		c->v[X(op)] &= c->v[Y(op)];
		break;
	case 0x3:
		c->v[X(op)] ^= c->v[Y(op)];
		break;
	case 0x4:
		opcode_8XY4(c, op);
		break;
	case 0x5:
		opcode_8XY5(c, op);
		break;
	case 0x6:
		opcode_8XY6(c, op);
		break;
	case 0x7:
		opcode_8XY7(c, op);
		break;
	case 0xe:
		opcode_8XYE(c, op);
		break;
	default:
		break;
	}
}

static void opcode_9XY0(chip8_t *c, u16 op)
{
	if (c->v[X(op)] != c->v[Y(op)])
		c->pc += 2;
}

static void opcode_ANNN(chip8_t *c, u16 op)
{
	c->idr_addr = KKK(op);
}

static void opcode_BNNN(chip8_t *c, u16 op)
{
	c->pc = NNN(op) + c->v[0x0];
	c->pc -= 2;
}

static void opcode_CXNN(chip8_t *c, u16 op)
{
	c->v[X(op)] = ((u8) (rand() % 256)) & KK(op);
}

static void opcode_DXYN(chip8_t *c, u16 op)
{
	const u16 x = X(op);
	const u16 y = Y(op);
	const u16 n = N(op);
	c->v[0xf] = 0x0;
	for (u16 w = 0; w < n; w++) {
		const u16 b = (w + c->v[y]) % 32;
		const u16 p = c->mem[c->idr_addr + w];
		for (u16 u = 0; u < 8; u++) {
			const u16 a = (u + c->v[x]) % 64;
			const u16 d = !!(p & (0x80 >> u));
			c->display[b * 64 + a] ^= d;
			c->v[0xf] |= d;
		}
	}
}

static void opcode_EX9E(chip8_t *c, u16 op)
{
	if (c->key[c->v[X(op)]])
		c->pc += 2;
}

static void opcode_EXA1(chip8_t *c, u16 op)
{
	if (!c->key[c->v[X(op)]])
		c->pc += 2;
}

static void opcode_EX__(chip8_t *c, u16 op)
{
	switch (KK(op)) {
	case 0x9e:
		opcode_EX9E(c, op);
		break;
	case 0xa1:
		opcode_EXA1(c, op);
		break;
	default:
		break;
	}
}

static void opcode_FX1E(chip8_t *c, u16 op)
{
	c->idr_addr += c->v[X(op)];
	c->v[0xf] = c->idr_addr > 0xfff;
}

static void opcode_FX0A(chip8_t *c, u16 op)
{
	for (u8 i = 0x0; i < 0x10; i++)
		if (c->key[i]) {
			c->v[X(op)] = i;
			return;
		}
	c->pc -= 2;
}

static void opcode_FX29(chip8_t *c, u16 op)
{
	c->idr_addr = (c->v[X(op)] & 0xf) * 5;
}

static void opcode_FX33(chip8_t *c, u16 op)
{
	c->mem[c->idr_addr] = c->v[X(op)] / 100;
	c->mem[c->idr_addr + 1] = c->v[X(op)] % 100 / 10;
	c->mem[c->idr_addr + 2] = c->v[X(op)] % 10;
}

static void opcode_FX55(chip8_t *c, u16 op)
{
	memcpy(c->mem + c->idr_addr, c->v, X(op) + 1);
	c->idr_addr += X(op) + 1;
}

static void opcode_FX65(chip8_t *c, u16 op)
{
	memcpy(c->v, c->mem + c->idr_addr, X(op) + 1);
	c->idr_addr += X(op) + 1;
}

static void opcode_FX__(chip8_t *c, u16 op)
{
	switch (KK(op)) {
	case 0x07:
		c->v[X(op)] = c->delay_timer;
		break;
	case 0x0a:
		opcode_FX0A(c, op);
		break;
	case 0x15:
		c->delay_timer = c->v[X(op)];
		break;
	case 0x18:
		c->sound_timer = c->v[X(op)];
		break;
	case 0x1e:
		opcode_FX1E(c, op);
		break;
	case 0x29:
		opcode_FX29(c, op);
		break;
	case 0x33:
		opcode_FX33(c, op);
		break;
	case 0x55:
		opcode_FX55(c, op);
		break;
	case 0x65:
		opcode_FX65(c, op);
		break;
	default:
		break;
	}
}

static void load_font_0(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0x90;
	mem[3] = 0x90;
	mem[4] = 0xF0;
}

static void load_font_1(u8 *mem)
{
	mem[0] = 0x20;
	mem[1] = 0x60;
	mem[2] = 0x20;
	mem[3] = 0x20;
	mem[4] = 0x70;
}

static void load_font_2(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x10;
	mem[2] = 0xF0;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

static void load_font_3(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x10;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0xF0;
}

static void load_font_4(u8 *mem)
{
	mem[0] = 0x90;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0x10;
}

static void load_font_5(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0xF0;
}

static void load_font_6(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x90;
	mem[4] = 0xF0;
}

static void load_font_7(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x10;
	mem[2] = 0x20;
	mem[3] = 0x40;
	mem[4] = 0x40;
}

static void load_font_8(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x90;
	mem[4] = 0xF0;
}

static void load_font_9(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0xF0;
}

static void load_font_A(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x90;
	mem[4] = 0x90;
}

static void load_font_B(u8 *mem)
{
	mem[0] = 0xE0;
	mem[1] = 0x90;
	mem[2] = 0xE0;
	mem[3] = 0x90;
	mem[4] = 0xE0;
}

static void load_font_C(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0x80;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

static void load_font_D(u8 *mem)
{
	mem[0] = 0xE0;
	mem[1] = 0x90;
	mem[2] = 0x90;
	mem[3] = 0x90;
	mem[4] = 0xE0;
}

static void load_font_E(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

static void load_font_F(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x80;
	mem[4] = 0x80;
}

static void load_system_font(chip8_t *c)
{
	load_font_0(c->mem + 0);
	load_font_1(c->mem + 5);
	load_font_2(c->mem + 10);
	load_font_3(c->mem + 15);
	load_font_4(c->mem + 20);
	load_font_5(c->mem + 25);
	load_font_6(c->mem + 30);
	load_font_7(c->mem + 35);
	load_font_8(c->mem + 40);
	load_font_9(c->mem + 45);
	load_font_A(c->mem + 50);
	load_font_B(c->mem + 55);
	load_font_C(c->mem + 60);
	load_font_D(c->mem + 65);
	load_font_E(c->mem + 70);
	load_font_F(c->mem + 75);
}

void chip8_load_system(chip8_t *c)
{
	memset(c->v, 0x00, sizeof(0[c->v]) * 0x10);
	memset(c->key, 0x00, sizeof(0[c->key]) * 0x10);
	memset(c->stack, 0x00, sizeof(0[c->stack]) * 0x10);
	memset(c->mem, 0x00, sizeof(0[c->mem]) * 0x1000);
	memset(c->display, 0x00, sizeof(0[c->display]) * 32 * 64);
	load_system_font(c);
	c->sp = 0;
	c->pc = 0x200;
	c->idr_addr = 0;
	c->sound_timer = 0;
	c->delay_timer = 0;
}

void chip8_step(chip8_t *c)
{
	const u16 op = c->mem[c->pc] << 8 | c->mem[c->pc + 1];
	switch ((op & 0xf000) >> 12) {
	case 0x0:
		opcode_0___(c, op);
		break;
	case 0x1:
		opcode_1NNN(c, op);
		break;
	case 0x2:
		opcode_2NNN(c, op);
		break;
	case 0x3:
		opcode_3XNN(c, op);
		break;
	case 0x4:
		opcode_4XNN(c, op);
		break;
	case 0x5:
		opcode_5XY0(c, op);
		break;
	case 0x6:
		opcode_6XNN(c, op);
		break;
	case 0x7:
		opcode_7XNN(c, op);
		break;
	case 0x8:
		opcode_8XY_(c, op);
		break;
	case 0x9:
		opcode_9XY0(c, op);
		break;
	case 0xa:
		opcode_ANNN(c, op);
		break;
	case 0xb:
		opcode_BNNN(c, op);
		break;
	case 0xc:
		opcode_CXNN(c, op);
		break;
	case 0xd:
		opcode_DXYN(c, op);
		break;
	case 0xe:
		opcode_EX__(c, op);
		break;
	case 0xf:
		opcode_FX__(c, op);
		break;
	default:
		assert(false);
		break;
	}
	c->pc += 2;
}
