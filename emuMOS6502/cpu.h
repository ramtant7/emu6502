#pragma once

#include <stdint.h>
#include "memory.h"

#define MAX_RAM 64

struct state {
	uint32_t pc, s, a, x, y, p, n;
	uint32_t addr[MAX_RAM], val[MAX_RAM];
};

class Cpu {
public:
	Cpu(Memory *mem);
	void reset();
	int step();

	void load(const struct state* st);
	int compare(const struct state* want, uint32_t want_cycles, int cycles);

private:
	uint16_t pc;
	uint8_t a, x, y, sp, p;
	uint64_t cycles;
	Memory* mem;


	uint16_t read16(uint16_t addr);
	int execute(uint8_t op);
	int check(const char* name, uint32_t want, uint32_t got);
	void set_flag(uint8_t flag, int on);
	uint8_t set_nz(uint8_t v);
	uint16_t read16_in_page(uint16_t addr);
	uint8_t fetch();
	uint16_t fetch16();
	uint16_t indexed(uint16_t base, uint8_t index, int* crossed);


	void inx();
	void iny();
	void dex();
	void dey();

	uint8_t pull();
	void push(uint8_t v);
	void push_a();
	void pull_a();
	void push_p();
	void pull_p();

	void and_imm();
	void and_abs();
	void and_zp();
	void and_zpx();
	void and_izx();
	int and_izy();
	int and_abx();
	int and_aby();

	void or_imm();
	void or_abs();
	void or_zp();
	void or_zpx();
	void or_izx();
	int or_izy();
	int or_abx();
	int or_aby();

	void xor_imm();
	void xor_abs();
	void xor_zp();
	void xor_zpx();
	void xor_izx();
	int xor_izy();
	int xor_abx();
	int xor_aby();

	void lda_imm();
	void lda_abs();
	void lda_zp();
	void lda_zpx();
	void lda_izx();
	int lda_izy();
	int lda_abx();
	int lda_aby();

	void ldx_imm();
	void ldx_abs();
	void ldx_zp();
	void ldx_zpy();
	int ldx_aby();

	void ldy_imm();
	void ldy_abs();
	void ldy_zp();
	void ldy_zpx();
	int ldy_abx();

};