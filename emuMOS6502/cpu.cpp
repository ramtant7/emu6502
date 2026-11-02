#include <stdio.h>

#include "cpu.h"
#include "memory.h"

enum {
	FLAG_C = 0x01,
	FLAG_Z = 0x02,
	FLAG_I = 0x04,
	FLAG_D = 0x08,
	FLAG_B = 0x10,
	FLAG_U = 0x20,
	FLAG_V = 0x40,
	FLAG_N = 0x80,
};


Cpu::Cpu(Memory* mem) {
	this->mem = mem;
}


//                                          PUBLIC FUNCTIONS
void Cpu::reset() {
	a = 0;
	x = 0;
	y = 0;
	p = 0xFD;
	pc = read16(0xFFFC);
	cycles = 7;
}

int Cpu::step() {
	uint16_t start = pc;
	int n = execute(mem->read(pc++));

	if (n < 0) {
		pc = start;
		return -1;
	}
	cycles += (uint64_t)n;
	return n;
}

void Cpu::load(const struct state* st)
{
	pc = (uint16_t)st->pc;
	sp = (uint8_t)st->s;
	a = (uint8_t)st->a;
	x = (uint8_t)st->x;
	y = (uint8_t)st->y;
	p = (uint8_t)st->p;
	cycles = 0;
	for (uint32_t i = 0; i < st->n; i++)
		mem->write((uint16_t)st->addr[i], (uint8_t)st->val[i]);
}

int Cpu::compare(const struct state *want, uint32_t want_cycles, int cycles)
{
	int bad = 0;

	bad += check("PC", want->pc, pc);
	bad += check("SP", want->s, sp);
	bad += check("A", want->a, a);
	bad += check("X", want->x, x);
	bad += check("Y", want->y, y);
	bad += check("P", want->p, p);
	bad += check("cycles", want_cycles, (unsigned)cycles);
	for (unsigned i = 0; i < want->n; i++) {
		char name[16];
		snprintf(name, sizeof name, "[$%04X]", want->addr[i]);
		bad += check(name, want->val[i], mem->read((uint16_t)want->addr[i]));
	}
	return bad;
}



//                                          PRIVATE FUNCTIONS


void Cpu::set_flag(uint8_t flag, int on)
{
	if (on == 0) {                        //  (on ? flag : 0) если верно то первое иначе второе
		p = (uint8_t)((p & ~flag) | 0);
	}
	else
	{
		p = (uint8_t)((p & ~flag) | flag);
	}
}

uint8_t Cpu::set_nz(uint8_t v) {
	p = (uint8_t)((p & ~(FLAG_N | FLAG_Z)) | (v & FLAG_N) | (v ? 0 : FLAG_Z)); //(биты флагов N и Z сбросятся до 0) (смотрит на старший бит числа)
	return v;
}

void Cpu::inx() {
	x = set_nz((uint8_t)(x + 1));
}

void Cpu::iny() {
	y = set_nz((uint8_t)(y + 1));
}

void Cpu::dex() {
	x = set_nz((uint8_t)(x - 1));
}

void Cpu::dey() {
	y = set_nz((uint8_t)(y - 1));
}


uint16_t Cpu::read16(uint16_t addr)
{
	return (uint16_t)(this->mem->read(addr) | this->mem->read((uint16_t)(addr + 1)) << 8);
}

int Cpu::execute(uint8_t op) {
	int crossed = 0;
	
	switch (op) {
	case 0x18: set_flag(FLAG_C, 0); return 2; //CLC
	case 0x38: set_flag(FLAG_C, 1); return 2; //SEC
	case 0x58: set_flag(FLAG_I, 0); return 2; //CLI
	case 0x78: set_flag(FLAG_I, 1); return 2; //SEI
	case 0xD8: set_flag(FLAG_D, 0); return 2; //CLD
	case 0xF8: set_flag(FLAG_D, 1); return 2; //SED
	case 0xB8: set_flag(FLAG_V, 0); return 2; //CLV

	case 0xE8: inx(); return 2; //INX
	case 0xC8: iny(); return 2; //INY
	case 0xCA: dex(); return 2; //DEX
	case 0x88: dey(); return 2; //DEY

	case 0xEA: return 2; //NOP
// остальные инструкции

	default:
		return -1;
	}
}

int Cpu::check(const char* name, uint32_t want, uint32_t got)
{
	if (want == got)
		return 0;
	printf("  %s: want $%02X, got $%02X\n", name, want, got);
	return 1;
}