#include "mc6809.hpp"
#include <cstdio>

mc6809::mc6809()
{
	opcodes[0] = &mc6809::lda;
	opcodes[1] = &mc6809::sta;
}

bool mc6809::run(uint16_t cycles)
{
	(this->*opcodes[0])();
	(this->*opcodes[1])();
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