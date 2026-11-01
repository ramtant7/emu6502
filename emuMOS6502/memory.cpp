#include "memory.h"

uint8_t Memory::read(uint16_t addr) 
{
	return this->mem[addr];
}

void Memory::write(uint16_t addr, uint8_t value)
{
	this->mem[addr] = value;
}