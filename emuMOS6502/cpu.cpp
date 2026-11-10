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

uint16_t Cpu::read16_in_page(uint16_t addr)
{
	uint16_t hi = (uint16_t)((addr & 0xFF00) | ((addr + 1) & 0xFF));
	return (uint16_t)(mem->read(addr) | mem->read(hi) << 8);
}

uint8_t Cpu::fetch()
{
	return mem->read(pc++);
}

uint16_t Cpu::fetch16()
{
	uint8_t lo = fetch();
	return (uint16_t)(lo | fetch() << 8);
}

uint16_t Cpu::indexed(uint16_t base, uint8_t index, int* crossed)
{
	uint16_t addr = (uint16_t)(base + index);
	*crossed = (base ^ addr) > 0xFF;
	return addr;
}



void Cpu::push(uint8_t v)
{
	mem->write((uint16_t)(0x100 | sp--), v);
}

uint8_t Cpu::pull()
{
	return mem->read((uint16_t)(0x100 | ++sp));
}

void Cpu::push_a() 
{
	push(a);
}

void Cpu::pull_a() 
{
	a = set_nz(pull());
}

void Cpu::push_p()
{
	push(p | FLAG_B | FLAG_U);
}

void Cpu::pull_p()
{
	p = (uint8_t)((pull() & ~FLAG_B) | FLAG_U);
}

// AND

void Cpu::and_imm() 
{
	a = set_nz(a & fetch());
}

void Cpu::and_abs() 
{
	a = set_nz(a & mem->read(fetch16()));
}

void Cpu::and_zp() 
{
	a = set_nz(a & mem->read(fetch()));
}

void Cpu::and_zpx() 
{
	uint16_t addr = (uint8_t)(fetch() + x);

	a = set_nz(a & mem->read(addr));
}

void Cpu::and_izx()
{
	uint16_t addr = read16_in_page((uint8_t)(fetch() + x));

	a = set_nz(a & mem->read(addr));
}

int Cpu::and_izy()
{
	int crossed;

	uint16_t addr = indexed(read16_in_page(fetch()), y, &crossed);

	a = set_nz(a & mem->read(addr));

	return crossed;
}

int Cpu::and_abx()
{
	int crossed;

	uint16_t addr = indexed(fetch16(), x, &crossed);

	a = set_nz(a & mem->read(addr));

	return crossed;
}

int Cpu::and_aby()
{
	int crossed;

	uint16_t addr = indexed(fetch16(), y, &crossed);

	a = set_nz(a & mem->read(addr));

	return crossed;
}

// OR

void Cpu::or_imm()
{
	a = set_nz(a | fetch());
}

void Cpu::or_abs()
{
	a = set_nz(a | mem->read(fetch16()));
}

void Cpu::or_zp()
{
	a = set_nz(a | mem->read(fetch()));
}

void Cpu::or_zpx()
{
	uint16_t addr = (uint8_t)(fetch() + x);

	a = set_nz(a | mem->read(addr));
}

void Cpu::or_izx()
{
	uint16_t addr = read16_in_page((uint8_t)(fetch() + x));

	a = set_nz(a | mem->read(addr));
}

int Cpu::or_izy()
{
	int crossed;

	uint16_t addr = indexed(read16_in_page(fetch()), y, &crossed);

	a = set_nz(a | mem->read(addr));

	return crossed;
}

int Cpu::or_abx()
{
	int crossed;

	uint16_t addr = indexed(fetch16(), x, &crossed);

	a = set_nz(a | mem->read(addr));

	return crossed;
}

int Cpu::or_aby()
{
	int crossed;

	uint16_t addr = indexed(fetch16(), y, &crossed);

	a = set_nz(a | mem->read(addr));

	return crossed;
}

// XOR

void Cpu::xor_imm()
{
	a = set_nz(a ^ fetch());
}

void Cpu::xor_abs()
{
	a = set_nz(a ^ mem->read(fetch16()));
}

void Cpu::xor_zp()
{
	a = set_nz(a ^ mem->read(fetch()));
}

void Cpu::xor_zpx()
{
	uint16_t addr = (uint8_t)(fetch() + x);

	a = set_nz(a ^ mem->read(addr));
}

void Cpu::xor_izx()
{
	uint16_t addr = read16_in_page((uint8_t)(fetch() + x));

	a = set_nz(a ^ mem->read(addr));
}

int Cpu::xor_izy()
{
	int crossed;

	uint16_t addr = indexed(read16_in_page(fetch()), y, &crossed);

	a = set_nz(a ^ mem->read(addr));

	return crossed;
}

int Cpu::xor_abx()
{
	int crossed;

	uint16_t addr = indexed(fetch16(), x, &crossed);

	a = set_nz(a ^ mem->read(addr));

	return crossed;
}

int Cpu::xor_aby()
{
	int crossed;

	uint16_t addr = indexed(fetch16(), y, &crossed);

	a = set_nz(a ^ mem->read(addr));

	return crossed;
}

// LDA

void Cpu::lda_imm()
{
	a = set_nz(fetch());
}

void Cpu::lda_abs()
{
	a = set_nz(mem->read(fetch16()));
}

void Cpu::lda_zp()
{
	a = set_nz(mem->read(fetch()));
}

void Cpu::lda_zpx()
{
	uint16_t addr = (uint8_t)(fetch() + x);

	a = set_nz(mem->read(addr));
}

void Cpu::lda_izx()
{
	uint16_t addr = read16_in_page((uint8_t)(fetch() + x));

	a = set_nz(mem->read(addr));
}

int Cpu::lda_izy()
{
	int crossed;

	uint16_t addr = indexed(read16_in_page(fetch()), y, &crossed);

	a = set_nz(mem->read(addr));

	return crossed;
}

int Cpu::lda_abx()
{
	int crossed;

	uint16_t addr = indexed(fetch16(), x, &crossed);

	a = set_nz(mem->read(addr));

	return crossed;
}

int Cpu::lda_aby()
{
	int crossed;

	uint16_t addr = indexed(fetch16(), y, &crossed);

	a = set_nz(mem->read(addr));

	return crossed;
}

// LDX

void Cpu::ldx_imm()
{
	x = set_nz(fetch());
}

void Cpu::ldx_abs()
{
	x = set_nz(mem->read(fetch16()));
}

void Cpu::ldx_zp()
{
	x = set_nz(mem->read(fetch()));
}

void Cpu::ldx_zpy()
{
	uint16_t addr = (uint8_t)(fetch() + y);

	x = set_nz(mem->read(addr));
}

int Cpu::ldx_aby()
{
	int crossed;

	uint16_t addr = indexed(fetch16(), y, &crossed);

	x = set_nz(mem->read(addr));

	return crossed;
}

// LDY

void Cpu::ldy_imm()
{
	y = set_nz(fetch());
}

void Cpu::ldy_abs()
{
	y = set_nz(mem->read(fetch16()));
}

void Cpu::ldy_zp()
{
	y = set_nz(mem->read(fetch()));
}

void Cpu::ldy_zpx()
{
	uint16_t addr = (uint8_t)(fetch() + x);

	y = set_nz(mem->read(addr));
}

int Cpu::ldy_abx()
{
	int crossed;

	uint16_t addr = indexed(fetch16(), x, &crossed);

	y = set_nz(mem->read(addr));

	return crossed;
}








uint16_t Cpu::read16(uint16_t addr)
{
	return (uint16_t)(this->mem->read(addr) | this->mem->read((uint16_t)(addr + 1)) << 8);
}

int Cpu::execute(uint8_t op) {
	int crossed = 0;
	
	switch (op) {

	case 0x48: push_a(); return 3; //PHA
	case 0x68: pull_a(); return 4; //PLA
	case 0x08: push_p(); return 3; //PHP
	case 0x28: pull_p(); return 4; //PLP

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

	case 0x29: and_imm(); return 2; //AND
	case 0x2D: and_abs(); return 4; //AND
	case 0x25: and_zp(); return 3; //AND
	case 0x35: and_zpx(); return 4; //AND
	case 0x21: and_izx(); return 6; //AND
	case 0x31: return 5 + and_izy(); //AND
	case 0x3D: return 4 + and_abx(); //AND
	case 0x39: return 4 + and_aby(); //AND

	case 0x09: or_imm(); return 2; //ORA
	case 0x0D: or_abs(); return 4; //ORA
	case 0x05: or_zp(); return 3; //ORA
	case 0x15: or_zpx(); return 4; //ORA
	case 0x01: or_izx(); return 6; //ORA
	case 0x11: return 5 + or_izy(); //ORA
	case 0x1D: return 4 + or_abx(); //ORA
	case 0x19: return 4 + or_aby(); //ORA

	case 0x49: xor_imm(); return 2; //EOR
	case 0x4D: xor_abs(); return 4; //EOR
	case 0x45: xor_zp(); return 3; //EOR
	case 0x55: xor_zpx(); return 4; //EOR
	case 0x41: xor_izx(); return 6; //EOR
	case 0x51: return 5 + xor_izy(); //EOR
	case 0x5D: return 4 + xor_abx(); //EOR
	case 0x59: return 4 + xor_aby(); //EOR

	case 0xA9: lda_imm(); return 2; //LDA
	case 0xAD: lda_abs(); return 4; //LDA
	case 0xA5: lda_zp(); return 3; //LDA
	case 0xB5: lda_zpx(); return 4; //LDA
	case 0xA1: lda_izx(); return 6; //LDA
	case 0xB1: return 5 + lda_izy(); //LDA
	case 0xBD: return 4 + lda_abx(); //LDA
	case 0xB9: return 4 + lda_aby(); //LDA

	case 0xA2: ldx_imm(); return 2;
	case 0xAE: ldx_abs(); return 4;
	case 0xA6: ldx_zp(); return 3;
	case 0xB6: ldx_zpy(); return 4;
	case 0xBE: return 4 + ldx_aby();

	case 0xA0: ldy_imm(); return 2;
	case 0xAC: ldy_abs(); return 4;
	case 0xA4: ldy_zp(); return 3;
	case 0xB4: ldy_zpx(); return 4;
	case 0xBC: return 4 + ldy_abx();

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