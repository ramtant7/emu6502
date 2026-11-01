#pragma once

#include <stdint.h>

class Memory {
public:
	uint8_t read(uint16_t addr);
	void write(uint16_t addr, uint8_t value);

private:
	uint8_t mem[0x10000];
};