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



	void inx();
	void iny();
	void dex();
	void dey();
};