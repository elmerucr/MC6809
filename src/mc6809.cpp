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
	uint16_t effective_address;

	// fetch opcode
	opcode = (*read_8)(pc++);

	effective_address = (this->*addressing_modes[opcode])();
	(this->*opcodes[opcode])(effective_address);
	return false;
}

/*
 * Addressing modes
 */

uint16_t mc6809::inh()
{
	return 0;
}

uint16_t mc6809::imm()
{
	return pc++;
}

/*
 * instructions
 */

void mc6809::abx(uint16_t ea)
{
	//
}

void mc6809::andcc(uint16_t ea)
{
	//
}

void mc6809::asl(uint16_t ea)
{
	//
}

void mc6809::asr(uint16_t ea)
{
	//
}

void mc6809::beq(uint16_t ea)
{
	//
}

void mc6809::bge(uint16_t ea)
{
	//
}

void mc6809::bgt(uint16_t ea)
{
	//
}

void mc6809::bhi(uint16_t ea)
{
	//
}

void mc6809::bhs(uint16_t ea)
{
	//
}

void mc6809::ble(uint16_t ea)
{
	//
}

void mc6809::blo(uint16_t ea)
{
	//
}

void mc6809::bls(uint16_t ea)
{
	//
}

void mc6809::blt(uint16_t ea)
{
	//
}

void mc6809::bmi(uint16_t ea)
{
	//
}

void mc6809::bne(uint16_t ea)
{
	//
}

void mc6809::bpl(uint16_t ea)
{
	//
}

void mc6809::bra(uint16_t ea)
{
	//
}

void mc6809::brn(uint16_t ea)
{
	//
}

void mc6809::bvc(uint16_t ea)
{
	//
}

void mc6809::bvs(uint16_t ea)
{
	//
}

void mc6809::clr(uint16_t ea)
{
	//
}

void mc6809::com(uint16_t ea)
{
	//
}

void mc6809::cwai(uint16_t ea)
{
	//
}

void mc6809::daa(uint16_t ea)
{
	//
}

void mc6809::dec(uint16_t ea)
{
	//
}

void mc6809::exg(uint16_t ea)
{
	//
}

void mc6809::ill(uint16_t ea)
{
	// push all registers, load vector illegal opcode ....
}

void mc6809::inc(uint16_t ea)
{
	//
}

void mc6809::jmp(uint16_t ea)
{
	//
}

void mc6809::lbra(uint16_t ea)
{
	//
}

void mc6809::lbsr(uint16_t ea)
{
	//
}

void mc6809::lda(uint16_t ea)
{
	printf("lda()\n");
}

void mc6809::leax(uint16_t ea)
{
	//
}

void mc6809::leay(uint16_t ea)
{
	//
}

void mc6809::leas(uint16_t ea)
{
	//
}

void mc6809::leau(uint16_t ea)
{
	//
}

void mc6809::lsr(uint16_t ea)
{
	//
}

void mc6809::mul(uint16_t ea)
{
	//
}

void mc6809::neg(uint16_t ea)
{
	//
}

void mc6809::nop(uint16_t ea)
{
	//
}

void mc6809::orcc(uint16_t ea)
{
	//
}

void mc6809::page2(uint16_t ea)
{
	//
}

void mc6809::page3(uint16_t ea)
{
	//
}

void mc6809::pshs(uint16_t ea)
{
	//
}

void mc6809::pshu(uint16_t ea)
{
	//
}

void mc6809::puls(uint16_t ea)
{
	//
}

void mc6809::pulu(uint16_t ea)
{
	//
}

void mc6809::rol(uint16_t ea)
{
	//
}

void mc6809::ror(uint16_t ea)
{
	//
}

void mc6809::rts(uint16_t ea)
{
	//
}

void mc6809::sex(uint16_t ea)
{
	//
}

void mc6809::sta(uint16_t ea)
{
	printf("sta()\n");
}

void mc6809::swi(uint16_t ea)
{
	//
}

void mc6809::sync(uint16_t ea)
{
	//
}

void mc6809::tfr(uint16_t ea)
{
	//
}

void mc6809::tst(uint16_t ea)
{
	//
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
