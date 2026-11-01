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




uint16_t Cpu::read16(uint16_t addr)
{
	return (uint16_t)(this->mem->read(addr) | this->mem->read((uint16_t)(addr + 1)) << 8);
}

int Cpu::execute(uint8_t op) {
	int crossed = 0;
	
	switch (op) {

// остальные инструкции

	default:
		return -1;
	}
}