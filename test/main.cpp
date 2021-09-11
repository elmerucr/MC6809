#include "mc6809.hpp"

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
	mc6809 cpu(read, write);
	cpu.run(0);
	return 0;
}
