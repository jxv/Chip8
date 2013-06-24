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

typedef void (*opcodeFtpr) (struct chip8 *, u16);

void opcode0___(struct chip8 *, u16);
void opcode1NNN(struct chip8 *, u16);
void opcode2NNN(struct chip8 *, u16);
void opcode3XNN(struct chip8 *, u16);
void opcode4XNN(struct chip8 *, u16);
void opcode5XY0(struct chip8 *, u16);
void opcode6XNN(struct chip8 *, u16);
void opcode7XNN(struct chip8 *, u16);
void opcode8XY_(struct chip8 *, u16);
void opcode9XY0(struct chip8 *, u16);
void opcodeANNN(struct chip8 *, u16);
void opcodeBNNN(struct chip8 *, u16);
void opcodeCXNN(struct chip8 *, u16);
void opcodeDXYN(struct chip8 *, u16);
void opcodeEX__(struct chip8 *, u16);
void opcodeFX__(struct chip8 *, u16);

void opcode00E0(struct chip8 *, u16);
void opcode00EE(struct chip8 *, u16);
void opcode8XY4(struct chip8 *, u16);
void opcode8XY5(struct chip8 *, u16);
void opcode8XY6(struct chip8 *, u16);
void opcode8XY7(struct chip8 *, u16);
void opcode8XYE(struct chip8 *, u16);
void opcodeEX9E(struct chip8 *, u16);
void opcodeEXA1(struct chip8 *, u16);
void opcodeFX0A(struct chip8 *, u16);
void opcodeFX1E(struct chip8 *, u16);
void opcodeFX29(struct chip8 *, u16);
void opcodeFX33(struct chip8 *, u16);
void opcodeFX55(struct chip8 *, u16);
void opcodeFX65(struct chip8 *, u16);

void ldSysFnt(struct chip8 *);

opcodeFtpr opcodes[0x10] = {
	&opcode0___,
	&opcode1NNN,
	&opcode2NNN,
	&opcode3XNN,
	&opcode4XNN,
	&opcode5XY0,
	&opcode6XNN,
	&opcode7XNN,
	&opcode8XY_,
	&opcode9XY0,
	&opcodeANNN,
	&opcodeBNNN,
	&opcodeCXNN,
	&opcodeDXYN,
	&opcodeEX__,
	&opcodeFX__
};

void ldSysData(struct chip8 *c)
{
	int i;

	for (i = 0; i < 0x10; i++) {
		c->v[i] = 0x0;
		c->key[i] = 0x0;
		c->stk[i] = 0x0;
	}
	for (i = 0; i < 0x1000; i++) {
		c->mem[i] = 0x0;
	}
	for (i = 0; i < 32 * 64; i++) {
		c->disp[i] = 0x0;
	}
	ldSysFnt(c);
	c->sp = 0;
	c->pc = 0x200;
	c->i = 0;
	c->st = 0;
	c->dt = 0;
}

void timestep(struct chip8 *c)
{
	if (c->dt > 0) {
		c->dt--;
	}
	if (c->st > 0) {
		c->st--;
	}
}

void step(struct chip8 *c)
{
	u16 op;

	op = c->mem[c->pc] << 8 | c->mem[c->pc + 1];
/*	
	printf("op: %04x  pc: %03x  ", op, c->pc);
	printf("i: %03x  sp: %01x  ", c->i, c->sp);
	printf("v: %02x %02x %02x %02x %02x %02x %02x %02x ",
			c->v[0], c->v[1], c->v[2], c->v[3],
			c->v[4], c->v[5], c->v[6], c->v[7]);
	printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
			c->v[8], c->v[9], c->v[10], c->v[11],
			c->v[12], c->v[13], c->v[14], c->v[15]);
*/
	(*opcodes[(op & 0xf000) >> 12]) (c, op);
	c->pc += 2;

}

void opcode0___(struct chip8 *c, u16 op)
{
	switch (op & 0xff) {
	case 0xe0:
		opcode00E0(c, op);
		break;
	case 0xee:
		opcode00EE(c, op);
		break;
	default:
		break;		/* no rca */
	}
}

void opcode00E0(struct chip8 *c, u16 op)
{
	int x, y;
	for (y = 0; y < 32; y++) {
		for (x = 0; x < 64; x++) {
			c->disp[y * 64 + x] = 0;
		}
	}
}

void opcode00EE(struct chip8 *c, u16 op)
{
	c->sp--;
	c->pc = c->stk[c->sp];
}

void opcode1NNN(struct chip8 *c, u16 op)
{
	c->pc = NNN(op);
	c->pc -= 2;
}

void opcode2NNN(struct chip8 *c, u16 op)
{
	c->stk[c->sp] = c->pc;
	c->sp++;
	c->pc = NNN(op);
}

void opcode3XNN(struct chip8 *c, u16 op)
{
	if (c->v[X(op)] == KK(op)) {
		c->pc += 2;
	}
}

void opcode4XNN(struct chip8 *c, u16 op)
{
	if (c->v[X(op)] != KK(op)) {
		c->pc += 2;
	}
}

void opcode5XY0(struct chip8 *c, u16 op)
{
	if (c->v[X(op)] == c->v[Y(op)]) {
		c->pc += 2;
	}
}

void opcode6XNN(struct chip8 *c, u16 op)
{
	c->v[X(op)] = NN(op);
}

void opcode7XNN(struct chip8 *c, u16 op)
{
	c->v[X(op)] += NN(op);
}

void opcode8XY_(struct chip8 *c, u16 op)
{
	switch (K(op)) {
	case 0x0: c->v[X(op)] = c->v[Y(op)]; break;
	case 0x1: c->v[X(op)] |= c->v[Y(op)]; break;
	case 0x2: c->v[X(op)] &= c->v[Y(op)]; break;
	case 0x3: c->v[X(op)] ^= c->v[Y(op)]; break;
	case 0x4: opcode8XY4(c, op); break;
	case 0x5: opcode8XY5(c, op); break;
	case 0x6: opcode8XY6(c, op); break;
	case 0x7: opcode8XY7(c, op); break;
	case 0xe: opcode8XYE(c, op); break;
	default: break;
	}
}

void opcode8XY4(struct chip8 *c, u16 op)
{
	u16 vx, vy, res;

	vx = c->v[X(op)];
	vy = c->v[Y(op)];
	res = vx + vy;
	c->v[0xf] = res > 255;
	c->v[X(op)] = res & 0xff;
}

void opcode8XY5(struct chip8 *c, u16 op)
{
	u16 vx, vy, res;

	vx = c->v[X(op)];
	vy = c->v[Y(op)];
	res = vx - vy;
	c->v[0xf] = vx >= vy;
	c->v[X(op)] = res;
}

void opcode8XY6(struct chip8 *c, u16 op)
{
	c->v[0xf] = c->v[Y(op)] & 0x1;
	c->v[X(op)] = c->v[Y(op)] >> 1;
}

void opcode8XY7(struct chip8 *c, u16 op)
{
	u16 vx, vy, res;

	vx = c->v[X(op)];
	vy = c->v[Y(op)];
	res = vy - vx;
	c->v[0xf] = vy >= vx;
	c->v[X(op)] = res;
}

void opcode8XYE(struct chip8 *c, u16 op)
{
	c->v[0xf] = c->v[Y(op)] >> 7;
	c->v[X(op)] = c->v[Y(op)] << 1;
}

void opcode9XY0(struct chip8 *c, u16 op)
{
	if (c->v[X(op)] != c->v[Y(op)]) {
		c->pc += 2;
	}
}

void opcodeANNN(struct chip8 *c, u16 op)
{
	c->i = KKK(op);
}

void opcodeBNNN(struct chip8 *c, u16 op)
{
	c->pc = NNN(op) + c->v[0x0];
	c->pc -= 2;
}

void opcodeCXNN(struct chip8 *c, u16 op)
{
	c->v[X(op)] = ((u8) (rand() % 256)) & KK(op);
}

void opcodeDXYN(struct chip8 *c, u16 op)
{
	u16 x, y, n, p, u, w, d, a, b;

	x = X(op);
	y = Y(op);
	n = N(op);

	c->v[0xf] = 0x0;
	for (w = 0; w < n; w++) {
		b = (w + c->v[y]) % 32;
		p = c->mem[c->i + w];
		for (u = 0; u < 8; u++) {
			a = (u + c->v[x]) % 64;
			d = ! !(p & (0x80 >> u));
			c->disp[b * 64 + a] ^= d;
			c->v[0xf] |= d;
		}
	}
}

void opcodeEX__(struct chip8 *c, u16 op)
{
	switch (KK(op)) {
	case 0x9e: opcodeEX9E(c, op); break;
	case 0xa1: opcodeEXA1(c, op); break;
	default: break;
	}
}

void opcodeEX9E(struct chip8 *c, u16 op)
{
	if (c->key[c->v[X(op)]]) {
		c->pc += 2;
	}
}

void opcodeEXA1(struct chip8 *c, u16 op)
{
	if (!c->key[c->v[X(op)]]) {
		c->pc += 2;
	}
}

void opcodeFX__(struct chip8 *c, u16 op)
{
	switch (KK(op)) {
	case 0x07: c->v[X(op)] = c->dt;	break;
	case 0x0a: opcodeFX0A(c, op); break;
	case 0x15: c->dt = c->v[X(op)]; break;
	case 0x18: c->st = c->v[X(op)];	break;
	case 0x1e: opcodeFX1E(c, op); break;
	case 0x29: opcodeFX29(c, op); break;
	case 0x33: opcodeFX33(c, op); break;
	case 0x55: opcodeFX55(c, op); break;
	case 0x65: opcodeFX65(c, op); break;
	default: break;
	}
}

void opcodeFX1E(struct chip8 *c, u16 op)
{
	c->i += c->v[X(op)];
	c->v[0xf] = c->i > 0xfff;
}

void opcodeFX0A(struct chip8 *c, u16 op)
{
	u8 i;
	for (i = 0x0; i < 0x10; i++) {
		if (c->key[i]) {
			c->v[X(op)] = i;
			return;
		}
	}
	c->pc -= 2;
}

void opcodeFX29(struct chip8 *c, u16 op)
{
	c->i = (c->v[X(op)] & 0xf) * 5;
}

void opcodeFX33(struct chip8 *c, u16 op)
{
	c->mem[c->i] = c->v[X(op)] / 100;
	c->mem[c->i + 1] = c->v[X(op)] % 100 / 10;
	c->mem[c->i + 2] = c->v[X(op)] % 10;
}

void opcodeFX55(struct chip8 *c, u16 op)
{
	memcpy(c->mem + c->i, c->v, X(op) + 1);
	c->i += X(op) + 1;
}

void opcodeFX65(struct chip8 *c, u16 op)
{
	memcpy(c->v, c->mem + c->i, X(op) + 1);
	c->i += X(op) + 1;
}

void ldFnt0(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0x90;
	mem[3] = 0x90;
	mem[4] = 0xF0;
}

void ldFnt1(u8 * mem)
{
	mem[0] = 0x20;
	mem[1] = 0x60;
	mem[2] = 0x20;
	mem[3] = 0x20;
	mem[4] = 0x70;
}

void ldFnt2(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x10;
	mem[2] = 0xF0;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

void ldFnt3(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x10;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0xF0;
}

void ldFnt4(u8 * mem)
{
	mem[0] = 0x90;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0x10;
}

void ldFnt5(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0xF0;
}

void ldFnt6(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x90;
	mem[4] = 0xF0;
}

void ldFnt7(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x10;
	mem[2] = 0x20;
	mem[3] = 0x40;
	mem[4] = 0x40;
}

void ldFnt8(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x90;
	mem[4] = 0xF0;
}

void ldFnt9(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x10;
	mem[4] = 0xF0;
}

void ldFntA(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x90;
	mem[2] = 0xF0;
	mem[3] = 0x90;
	mem[4] = 0x90;
}

void ldFntB(u8 * mem)
{
	mem[0] = 0xE0;
	mem[1] = 0x90;
	mem[2] = 0xE0;
	mem[3] = 0x90;
	mem[4] = 0xE0;
}

void ldFntC(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0x80;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

void ldFntD(u8 * mem)
{
	mem[0] = 0xE0;
	mem[1] = 0x90;
	mem[2] = 0x90;
	mem[3] = 0x90;
	mem[4] = 0xE0;
}

void ldFntE(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x80;
	mem[4] = 0xF0;
}

void ldFntF(u8 * mem)
{
	mem[0] = 0xF0;
	mem[1] = 0x80;
	mem[2] = 0xF0;
	mem[3] = 0x80;
	mem[4] = 0x80;
}

void ldSysFnt(struct chip8 *c)
{
	ldFnt0(c->mem + 0);
	ldFnt1(c->mem + 5);
	ldFnt2(c->mem + 10);
	ldFnt3(c->mem + 15);
	ldFnt4(c->mem + 20);
	ldFnt5(c->mem + 25);
	ldFnt6(c->mem + 30);
	ldFnt7(c->mem + 35);
	ldFnt8(c->mem + 40);
	ldFnt9(c->mem + 45);
	ldFntA(c->mem + 50);
	ldFntB(c->mem + 55);
	ldFntC(c->mem + 60);
	ldFntD(c->mem + 65);
	ldFntE(c->mem + 70);
	ldFntF(c->mem + 75);
}

