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

static void nop(u16 op, chip8_t *c)
{
#ifdef NDEBUG
	(void)c;
	(void)op;
#else
	fprintf(stderr, "addr:0x%.4x nop:0x%.4x\n", c->pc, op);
	exit(EXIT_FAILURE);
#endif
}

static void opcode_00e0(u16 op, chip8_t *c)
{
	memset(c->display, 0x00, sizeof(*c->display) * 32 * 64);
	c->pc += 2;
}

static void opcode_00ee(u16 op, chip8_t *c)
{
	c->sp--;
	c->pc = c->stack[c->sp];
}

static void opcode_0___(u16 op, chip8_t *c)
{
	switch (op & 0xff) {
	case 0xe0:
		opcode_00e0(op, c);
		break;
	case 0xee:
		opcode_00ee(op, c);
		break;
	default:
		nop(op, c);
		break;	/* no rca */
	}
}

static void opcode_1nnn(u16 op, chip8_t *c)
{
	c->pc = NNN(op);
}

static void opcode_2nnn(u16 op, chip8_t *c)
{
	c->stack[c->sp] = c->pc + 2;
	c->sp++;
	c->pc = NNN(op);
}

static void opcode_3xnn(u16 op, chip8_t *c)
{
	if (c->v[X(op)] == KK(op))
		c->pc += 4;
	else
		c->pc += 2;
}

static void opcode_4xnn(u16 op, chip8_t *c)
{
	if (c->v[X(op)] != KK(op))
		c->pc += 4;
	else
		c->pc += 2;
}

static void opcode_5xy0(u16 op, chip8_t *c)
{
	if (c->v[X(op)] == c->v[Y(op)])
		c->pc += 4;
	else
		c->pc += 2;
}

static void opcode_5xy_(u16 op, chip8_t *c)
{
	((op & 0x000f) == 0x0000 ? opcode_5xy0 : nop)(op, c);
}

static void opcode_6xnn(u16 op, chip8_t *c)
{
	c->v[X(op)] = NN(op);
	c->pc += 2;
}

static void opcode_7xnn(u16 op, chip8_t *c)
{
	c->v[X(op)] += NN(op);
	c->pc += 2;
}

static void opcode_8xy0(u16 op, chip8_t *c)
{
	c->v[X(op)] = c->v[Y(op)];
	c->pc += 2;
}

static void opcode_8xy1(u16 op, chip8_t *c)
{
	c->v[X(op)] |= c->v[Y(op)];
	c->pc += 2;
}

static void opcode_8xy2(u16 op, chip8_t *c)
{
	c->v[X(op)] &= c->v[Y(op)];
	c->pc += 2;
}

static void opcode_8xy3(u16 op, chip8_t *c)
{
	c->v[X(op)] ^= c->v[Y(op)];
	c->pc += 2;
}

static void opcode_8xy4(u16 op, chip8_t *c)
{
	c->v[0xf] = 255 < (u16)c->v[X(op)] + (u16)c->v[Y(op)];
	c->v[X(op)] = c->v[X(op)] + c->v[Y(op)];
	c->pc += 2;
}

static void opcode_8xy5(u16 op, chip8_t *c)
{
	c->v[0xf] = c->v[X(op)] >= c->v[Y(op)];
	c->v[X(op)] -= c->v[Y(op)];
	c->pc += 2;
}

static void opcode_8xy6(u16 op, chip8_t *c)
{
	c->v[0xf] = c->v[X(op)] & 0x01;
	c->v[X(op)] >>= 1;
	c->pc += 2;
}

static void opcode_8xy7(u16 op, chip8_t *c)
{
	c->v[0xf] = c->v[Y(op)] > c->v[X(op)];
	c->v[X(op)] = c->v[Y(op)] - c->v[X(op)];
	c->pc += 2;
}

static void opcode_8xye(u16 op, chip8_t *c)
{
	c->v[0xf] = (c->v[X(op)] & 0x80) >> 7;
	c->v[X(op)] <<= 1;
	c->pc += 2;
}

static void opcode_8xy_(u16 op, chip8_t *c)
{
	static void (*opcode_8___[0x10])(u16, chip8_t *) = {
		[0x0] = opcode_8xy0,
		[0x1] = opcode_8xy1,
		[0x2] = opcode_8xy2,
		[0x3] = opcode_8xy3,
		[0x4] = opcode_8xy4,
		[0x5] = opcode_8xy5,
		[0x6] = opcode_8xy6,
		[0x7] = opcode_8xy7,
		[0x8] = nop,
		[0x9] = nop,
		[0xa] = nop,
		[0xb] = nop,
		[0xc] = nop,
		[0xd] = nop,
		[0xe] = opcode_8xye,
		[0xf] = nop,
	};
	opcode_8___[op & 0x000f](op, c);
}

static void opcode_9xy0(u16 op, chip8_t *c)
{
	if (c->v[X(op)] != c->v[Y(op)])
		c->pc += 4;
	else
		c->pc += 2;
}

static void opcode_annn(u16 op, chip8_t *c)
{
	c->idr_addr = KKK(op);
	c->pc += 2;
}

static void opcode_bnnn(u16 op, chip8_t *c)
{
	c->pc = (u16)NNN(op) + (u16)c->v[0x0];
}

static void opcode_cxnn(u16 op, chip8_t *c)
{
	c->v[X(op)] = (u8)rand() & KK(op);
	c->pc += 2;
}

static void opcode_dxyn(u16 op, chip8_t *c)
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
	c->pc += 2;
}

static void opcode_ex9e(u16 op, chip8_t *c)
{
	if (c->key[c->v[X(op)]])
		c->pc += 4;
	else
		c->pc += 2;
}

static void opcode_exa1(u16 op, chip8_t *c)
{
	if (!c->key[c->v[X(op)]])
		c->pc += 4;
	else
		c->pc += 2;
}

static void opcode_ex__(u16 op, chip8_t *c)
{
	switch (KK(op)) {
	case 0x9e:
		opcode_ex9e(op, c);
		break;
	case 0xa1:
		opcode_exa1(op, c);
		break;
	default:
		nop(op, c);
		break;
	}
}

static void opcode_fx07(u16 op, chip8_t *c)
{
	c->v[X(op)] = c->delay_timer;
	c->pc += 2;
}

static void opcode_fx0a(u16 op, chip8_t *c)
{
	for (u8 i = 0x0; i < 0x10; i++)
		if (c->key[i]) {
			c->v[X(op)] = i;
			c->pc += 2;
			return;
		}
}

static void opcode_fx15(u16 op, chip8_t *c)
{
	c->delay_timer = c->v[X(op)];
	c->pc += 2;
}


static void opcode_fx18(u16 op, chip8_t *c)
{
	c->sound_timer = c->v[X(op)];
	c->pc += 2;
}

static void opcode_fx1e(u16 op, chip8_t *c)
{
	c->idr_addr += (u16)c->v[X(op)];
	/* c->v[0xf] = c->idr_addr > 0xfff; */
	c->pc += 2;
}

static void opcode_fx29(u16 op, chip8_t *c)
{
	c->idr_addr = (u16)c->v[X(op)] * 5;
	c->pc += 2;
}

static void opcode_fx33(u16 op, chip8_t *c)
{
	c->mem[c->idr_addr] = c->v[X(op)] / 100;
	c->mem[c->idr_addr + 1] = c->v[X(op)] % 100 / 10;
	c->mem[c->idr_addr + 2] = c->v[X(op)] % 10;
	c->pc += 2;
}

static void opcode_fx55(u16 op, chip8_t *c)
{
	memcpy(c->mem + c->idr_addr, c->v, X(op) + 1);
	c->idr_addr += X(op) + 1;
	c->pc += 2;
}

static void opcode_fx65(u16 op, chip8_t *c)
{
	memcpy(c->v, c->mem + c->idr_addr, X(op) + 1);
	c->idr_addr += X(op) + 1;
	c->pc += 2;
}

static void opcode_fx__(u16 op, chip8_t *c)
{
	switch (KK(op)) {
	case 0x07:
		opcode_fx07(op, c);
		break;
	case 0x0a:
		opcode_fx0a(op, c);
		break;
	case 0x15:
		opcode_fx15(op, c);
		break;
	case 0x18:
		opcode_fx18(op, c);
		break;
	case 0x1e:
		opcode_fx1e(op, c);
		break;
	case 0x29:
		opcode_fx29(op, c);
		break;
	case 0x33:
		opcode_fx33(op, c);
		break;
	case 0x55:
		opcode_fx55(op, c);
		break;
	case 0x65:
		opcode_fx65(op, c);
		break;
	default:
		nop(op, c);
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

static void load_font_a(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x90;
	mem[4] = 0x90;
}

static void load_font_b(u8 *mem)
{
	mem[0] = 0xE0;
	mem[1] = 0x90;
	mem[2] = 0xE0;
	mem[3] = 0x90;
	mem[4] = 0xE0;
}

static void load_font_c(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0x80;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

static void load_font_d(u8 *mem)
{
	mem[0] = 0xE0;
	mem[1] = 0x90;
	mem[2] = 0x90;
	mem[3] = 0x90;
	mem[4] = 0xE0;
}

static void load_font_e(u8 *mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

static void load_font_f(u8 *mem)
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
	load_font_a(c->mem + 50);
	load_font_b(c->mem + 55);
	load_font_c(c->mem + 60);
	load_font_d(c->mem + 65);
	load_font_e(c->mem + 70);
	load_font_f(c->mem + 75);
}

void chip8_load_system(chip8_t *c)
{
	memset(c->v, 0x00, sizeof(*c->v) * 0x10);
	memset(c->key, 0x00, sizeof(*c->key) * 0x10);
	memset(c->stack, 0x00, sizeof(*c->stack) * 0x10);
	memset(c->mem, 0x00, sizeof(*c->mem) * 0x1000);
	memset(c->display, 0x00, sizeof(*c->display) * 32 * 64);
	load_system_font(c);
	c->sp = 0;
	c->pc = 0x200;
	c->idr_addr = 0;
	c->sound_timer = 0;
	c->delay_timer = 0;
}

void chip8_step(chip8_t *c)
{
	static void (*opcode_____[0x10])(u16, chip8_t *) = {
		[0x0] = opcode_0___,
		[0x1] = opcode_1nnn,
		[0x2] = opcode_2nnn,
		[0x3] = opcode_3xnn,
		[0x4] = opcode_4xnn,
		[0x5] = opcode_5xy_,
		[0x6] = opcode_6xnn,
		[0x7] = opcode_7xnn,
		[0x8] = opcode_8xy_,
		[0x9] = opcode_9xy0,
		[0xa] = opcode_annn,
		[0xb] = opcode_bnnn,
		[0xc] = opcode_cxnn,
		[0xd] = opcode_dxyn,
		[0xe] = opcode_ex__,
		[0xf] = opcode_fx__,
	};
	const u16 op = c->mem[c->pc] << 8 | c->mem[c->pc + 1];
#ifndef NDBEUG
	static int i = 0;
	fprintf(stderr, "%d: 0x%.4x: 0x%.4x:\n", i, c->pc, op);
	i++;
#endif
	opcode_____[(op & 0xf000) >> 12](op, c);
}
