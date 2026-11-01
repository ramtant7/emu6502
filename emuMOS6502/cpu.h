#pragma once

#include <stdint.h>
#include "memory.h"

class Cpu {
public:
	Cpu(Memory *mem);
	void reset();
	int step();

private:
	uint16_t pc;
	uint8_t a, x, y, sp, p;
	uint64_t cycles;
	Memory* mem;


	uint16_t read16(uint16_t addr);
	int execute(uint8_t op);
};