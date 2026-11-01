#include <stdio.h>

#include "memory.h"
#include "cpu.h"

Memory mem;
Cpu cpu(&mem);

//void main() {
//	Memory m1;
//	Memory m2;
//	Cpu c1(&m1);
//	m1.write(0xFFFC, 0x0F);
//	m1.write(0xFFFD, 0xF1);
//	c1.reset();
//
//	printf("m1: %x\nm2: %x\n", m1.read(0x1000), m2.read(0x1000));
//
//	m1.write(0x1000, 0x00);
//	m2.write(0x1000, 0x11);
//
//	printf("m1: %x\nm2: %x\n", m1.read(0x1000), m2.read(0x1000));
//}

enum result { PASSED, FAILED, SKIPPED };



/* 1: read, 0: end of file, -1: malformed */
static int read_state(FILE* f, struct state* st)
{
	int r = fscanf(f, "%u %u %u %u %u %u %u", &st->pc, &st->s, &st->a, &st->x, &st->y, &st->p, &st->n);

	if (r == EOF)
		return 0;

	if (r != 7 || st->n > MAX_RAM)
		return -1;

	for (uint32_t i = 0; i < st->n; i++)
		if (fscanf(f, "%u %u", &st->addr[i], &st->val[i]) != 2)
			return -1;

	return 1;
}



static enum result run_file(const char* path, uint32_t opcode)
{
	FILE* f = fopen(path, "rb");
	struct state init, want;
	uint32_t want_cycles;
	enum result res = PASSED;

	if (f==0) {
		//printf("%s: cannot open (run `make fetch-tests`)\n", path);
		return FAILED;
	}

	for (uint32_t n = 0;; n++) {
		int r = read_state(f, &init);
		if (r == 0)
			break;

		if (r < 0 || read_state(f, &want) != 1 || fscanf(f, "%u", &want_cycles) != 1) {
			printf("%s: malformed case %u\n", path, n);
			res = FAILED;
			break;
		}

		cpu.load(&init);
		int cycles = cpu.step();
		if (cycles < 0 && n == 0) {
			res = SKIPPED;
			break;
		}
		if (cpu.compare(&want, want_cycles, cycles)) {
			printf("opcode $%02X case %u failed, initial PC=$%04X SP=$%02X A=$%02X X=$%02X Y=$%02X P=$%02X\n",
				opcode, n, init.pc, init.s, init.a, init.x, init.y, init.p);
			res = FAILED;
			break;
		}
	}
	fclose(f);
	return res;
}


int main(int argc, char** argv)
{
	uint32_t count[3] = { 0 };

	if (argc != 2) {
		printf("usage: %s DIR\n", argv[0]);
		return 2;
	}

	for (uint32_t op = 0; op < 256; op++) {
		char path[4096];
		snprintf(path, sizeof path, "%s/%02x.txt", argv[1], op);
		count[run_file(path, op)]++;
	}

	printf("%u passed, %u failed, %u skipped\n", count[PASSED], count[FAILED], count[SKIPPED]);
	return 0;
}