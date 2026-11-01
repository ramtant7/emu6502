#include <stdio.h>

#include "memory.h"
#include "cpu.h"

void main() {
	Memory m1;
	Memory m2;
	Cpu c1(&m1);
	m1.write(0xFFFC, 0x0F);
	m1.write(0xFFFD, 0xF1);
	c1.reset();

	printf("m1: %x\nm2: %x\n", m1.read(0x1000), m2.read(0x1000));

	m1.write(0x1000, 0x00);
	m2.write(0x1000, 0x11);

	printf("m1: %x\nm2: %x\n", m1.read(0x1000), m2.read(0x1000));
}