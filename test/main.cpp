/*
 * main.cpp  -  part of MC6809
 *
 * (c)2021 elmerucr
 */

#include "mc6809.hpp"
#include <cstdio>

uint8_t memory[65536];

uint8_t read(uint16_t address)
{
	return memory[address];
}

void write(uint16_t address, uint8_t byte)
{
	memory[address] = byte;
}

int main()
{
	memory[0xfffe] = 0xc0;	// reset vector $c000
	memory[0xffff] = 0x00;

	memory[0xc000] = 0x16;	// lbra $c00a
	memory[0xc001] = 0x00;
	memory[0xc002] = 0x07;

	memory[0xc00a] = 0x20;	// bra $c010
	memory[0xc00b] = 0x04;

	mc6809 cpu(read, write);
	cpu.reset();
	cpu.status();
	cpu.run(0);
	cpu.status();
	cpu.run(0);
	cpu.status();
	return 0;
}
