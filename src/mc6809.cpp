#include "mc6809.hpp"
#include <cstdio>

/* Constructor arranges references ar and br to right positions in
 * dr register. Currently assumes host system is LITTLE ENDIAN.
 */
mc6809::mc6809(bus_read r, bus_write w) : a(*(((uint8_t *)&d)+1)), b(*((uint8_t *)&d))
{
	read_8 = (bus_read)r;
	write_8 = (bus_write)w;

	ccr = 0b00000000;
}

void mc6809::reset()
{
	/*
	 * For 6800 compatibility, direct page register defaults to
	 * zero after a reset.
	 */
	dpr = 0x00;

	/*
	 * irq's and firq's disabled after reset
	 */
	ccr |= (I_FLAG | F_FLAG);

	/*
	 * After reset, nmi fully disabled. Only after a first write
	 * to the system stackpointer enabled.
	 */
	nmi_blocked = true;

	/*
	 * Load program counter from vector
	 */
	pc = ((*read_8)(VECTOR_RESET)) << 8;
	pc |= (*read_8)(VECTOR_RESET+1);
}

bool mc6809::run(uint16_t cycles)
{
	uint8_t opcode;

	// fetch
	opcode = (*read_8)(pc++);

	// bogus
	(this->*opcodes[opcode])();
	return false;
}

void mc6809::ill()
{
	//
}

void mc6809::lda()
{
	printf("lda()\n");
}

void mc6809::neg()
{
	// jaja
}

void mc6809::sta()
{
	printf("sta()\n");
}

void mc6809::status()
{
	printf(" a:   $%02x\n", a);
	printf(" b:   $%02x\n", b);
	printf(" d: $%04x\n", d);
	printf("dp:   $%02x\n", dpr);
	printf("    efhinzvc\n");
	printf("cc: %c%c%c%c%c%c%c%c\n",
			ccr & E_FLAG ? '1' : '0',
			ccr & F_FLAG ? '1' : '0',
			ccr & H_FLAG ? '1' : '0',
			ccr & I_FLAG ? '1' : '0',
			ccr & N_FLAG ? '1' : '0',
			ccr & Z_FLAG ? '1' : '0',
			ccr & V_FLAG ? '1' : '0',
			ccr & C_FLAG ? '1' : '0');
	printf(" x: $%04x\n", xr);
	printf(" y: $%04x\n", yr);
	printf(" u: $%04x\n", us);
	printf(" s: $%04x\n", sp);
	printf("pc: $%04x\n", pc);
}
