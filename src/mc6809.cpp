#include "mc6809.hpp"
#include <cstdio>

/*
 * Constructor arranges references A and B to right positions in
 * D register. Currently assumes host system is LITTLE ENDIAN
 */
mc6809::mc6809(bus_read r, bus_write w) : A(*(((uint8_t *)&D)+1)), B(*((uint8_t *)&D))
{
	read_8 = (bus_read)r;
	write_8 = (bus_write)w;
	
	opcodes[0] = &mc6809::lda;
	opcodes[1] = &mc6809::sta;
}

void mc6809::reset()
{
	DP = 0x00;		// direct page register is 0, 6800 compatibility
	nmi_allowed = false;	// nmi's allowed after S is initialized
}

bool mc6809::run(uint16_t cycles)
{
	(this->*opcodes[0])();
	(this->*opcodes[1])();
	printf("%02x\n", (*read_8)(0xc000));
	(*write_8)(0xc000,0xae);
	printf("%02x\n", (*read_8)(0xc000));
	return false;
}

void mc6809::lda()
{
	printf("lda()\n");
}

void mc6809::sta()
{
	printf("sta()\n");
}