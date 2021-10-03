/*
 * mc6809.cpp  -  part of MC6809
 *
 * (c)2021 elmerucr
 */

#include "mc6809.hpp"
#include <cstdio>

/*
 * Constructor arranges references ar and br to right positions in
 * dr register. Currently assumes host system is LITTLE ENDIAN.
 */
mc6809::mc6809(bus_read r, bus_write w) : ac(*(((uint8_t *)&dr)+1)), br(*((uint8_t *)&dr))
{
	read_8 = (bus_read)r;
	write_8 = (bus_write)w;

	cc = 0b00000000;

	/*
	 * When NFI pins are not yet assigned, there needs to be a
	 * decent value (true).
	 */
	default_pin = true;
	nmi_line = &default_pin;
	firq_line = &default_pin;
	irq_line = &default_pin;
	old_nmi_line = true;

	cycles = 0;
}

void mc6809::reset()
{
	/*
	 * For 6800 compatibility, direct page register defaults to
	 * zero after a reset.
	 */
	dp = 0x00;

	/*
	 * firq and irq masked after reset
	 */
	cc |= (I_FLAG | F_FLAG);

	/*
	 * After reset, nmi is fully disabled. Only after a first write
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
	// fetch opcode (= first byte of instruction, possible for)
	// breakpoints later on
	uint8_t opcode = (*read_8)(pc++);
	cycles += cycles_page1[opcode];

	uint16_t effective_address = (this->*addressing_modes_page1[opcode])();
	(this->*opcodes_page1[opcode])(effective_address);
	return false;
}

/*
 * Addressing modes
 */
uint16_t mc6809::a_dir()
{
	return (dp << 8) | (*read_8)(pc++);
}

uint16_t mc6809::a_ih()
{
	return 0;
}

uint16_t mc6809::a_im()
{
	return pc++;
}

uint16_t mc6809::a_reb()
{
	uint16_t offset = (uint16_t)((int8_t)(*read_8)(pc++));
	return (uint16_t)(pc + offset);
}

uint16_t mc6809::a_rew()
{
	uint16_t offset = (*read_8)(pc++);
	offset = (offset << 8) | (*read_8)(pc++);
	return pc + offset;
}

uint16_t mc6809::a_idx()
{
	// to do
	return 0;
}

uint16_t mc6809::a_ext()
{
	// to do
	return 0;
}

uint16_t mc6809::a_no()
{
	// no mode @ illegal instruction
	return 0;
}

/*
 *  pc  dp ac br  xr   yr   us   sp  efhinzvc  N F I
 * c000 00 01:ae 0000 d0d0 0000 0ffc -*-*---- 11 1 1
 */
void mc6809::status(char *text_buffer)
{
	sprintf(text_buffer, " pc  dp ac br  xr   yr   us   sp  efhinzvc  N F I\n"
			"%04x %02x %02x:%02x "
			"%04x %04x %04x %04x "
			"%c%c%c%c%c%c%c%c "
			"%c%c %c %c\n",
			pc, dp, ac, br,
			xr, yr, us, sp,
			cc & E_FLAG ? '*' : '-',
			cc & F_FLAG ? '*' : '-',
			cc & H_FLAG ? '*' : '-',
			cc & I_FLAG ? '*' : '-',
			cc & N_FLAG ? '*' : '-',
			cc & Z_FLAG ? '*' : '-',
			cc & V_FLAG ? '*' : '-',
			cc & C_FLAG ? '*' : '-',
			old_nmi_line ? '1' : '0',
			*nmi_line ? '1' : '0',
			*firq_line ? '1' : '0',
			*irq_line ? '1' : '0');
}
