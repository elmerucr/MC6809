/*
 * mc6809.cpp  -  part of MC6809
 *
 * (C)2021-2022 elmerucr
 */

#include "mc6809.hpp"
#include <cstdio>

mc6809::mc6809(bus_read r, bus_write w)
{
	read_8 = (bus_read)r;
	write_8 = (bus_write)w;

	cc = 0b00000000;

	/*
	 * When NFI pins are not (yet) assigned, there needs to be a
	 * decent starting value (true).
	 */
	default_pin = true;
	nmi_line = &default_pin;
	firq_line = &default_pin;
	irq_line = &default_pin;

	cycles = 0;

	index_regs[0b00] = &xr;
	index_regs[0b01] = &yr;
	index_regs[0b10] = &us;
	index_regs[0b11] = &sp;

	breakpoint_array = NULL;
	breakpoint_array = new bool[65536];
	clear_breakpoints();
	
	printf("[MC6809] version %i.%i.%i (C)%i elmerucr\n",
	       MC6809_MAJOR_VERSION,
	       MC6809_MINOR_VERSION,
	       MC6809_BUILD,
	       MC6809_YEAR);
}

mc6809::~mc6809()
{
	printf("[MC6809] cleaning up\n");
	delete breakpoint_array;
}

void mc6809::reset()
{
	printf("[MC6809] resetting cpu\n");
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
	nmi_enabled = false;
	old_nmi_line = *nmi_line;

	/*
	 * Load program counter from vector
	 */
	pc = 0;
	pc = ((*read_8)(VECTOR_RESET)) << 8;
	pc |= (*read_8)(VECTOR_RESET+1);
}

uint8_t mc6809::execute()
{
	uint32_t old_cycles = cycles;
	
	if ((*nmi_line == false) && (old_nmi_line == true) && nmi_enabled) {
		nmi();
	} else if ((*firq_line == false) && is_f_flag_clear()) {
		firq();
	} else if ((*irq_line == false) && is_i_flag_clear()) {
		irq();
	} else {
		uint8_t opcode = (*read_8)(pc++);
		/*
		 * TODO: check for illegal opcode and start exception
		 */
		cycles += cycles_page1[opcode];
		bool am_legal;
		uint16_t effective_address = (this->*addressing_modes_page1[opcode])(&am_legal);
		(this->*opcodes_page1[opcode])(effective_address);
	}
	
	old_nmi_line = *nmi_line;
	return cycles - old_cycles;
}

void mc6809::toggle_breakpoint(uint16_t address)
{
	breakpoint_array[address] = !breakpoint_array[address];
}

void mc6809::clear_breakpoints()
{
	for (int i=0; i<65536; i++) {
		breakpoint_array[i] = false;
	}
}

void mc6809::nmi()
{
	push_sp(pc & 0x00ff);
	push_sp((pc & 0xff00) >> 8);
	push_sp(us & 0x00ff);
	push_sp((us & 0xff00) >> 8);
	push_sp(yr & 0x00ff);
	push_sp((yr & 0xff00) >> 8);
	push_sp(xr & 0x00ff);
	push_sp((xr & 0xff00) >> 8);
	push_sp(dp);
	push_sp(br);
	push_sp(ac);
	set_e_flag();
	push_sp(cc);
	set_i_flag();
	set_f_flag();
	pc = 0;
	pc = ((*read_8)(VECTOR_NMI)) << 8;
	pc |= (*read_8)(VECTOR_NMI+1);

	/*
	 * can't find this in the documentation
	 */
	cycles += 19;
}

void mc6809::firq()
{
	push_sp(pc & 0x00ff);
	push_sp((pc & 0xff00) >> 8);
	clear_e_flag();
	push_sp(cc);
	set_f_flag();
	set_i_flag();
	pc = 0;
	pc = ((*read_8)(VECTOR_FIRQ)) << 8;
	pc |= (*read_8)(VECTOR_FIRQ+1);

	/*
	 * can't find this in the documentation
	 */
	cycles += 10;
}

void mc6809::irq()
{
	push_sp(pc & 0x00ff);
	push_sp((pc & 0xff00) >> 8);
	push_sp(us & 0x00ff);
	push_sp((us & 0xff00) >> 8);
	push_sp(yr & 0x00ff);
	push_sp((yr & 0xff00) >> 8);
	push_sp(xr & 0x00ff);
	push_sp((xr & 0xff00) >> 8);
	push_sp(dp);
	push_sp(br);
	push_sp(ac);
	set_e_flag();
	push_sp(cc);
	set_i_flag();
	pc = 0;
	pc = ((*read_8)(VECTOR_IRQ)) << 8;
	pc |= (*read_8)(VECTOR_IRQ+1);

	/*
	 * can't find this in the documentation
	 */
	cycles += 19;
}

void mc6809::illegal_opcode()
{
	push_sp(pc & 0x00ff);
	push_sp((pc & 0xff00) >> 8);
	push_sp(us & 0x00ff);
	push_sp((us & 0xff00) >> 8);
	push_sp(yr & 0x00ff);
	push_sp((yr & 0xff00) >> 8);
	push_sp(xr & 0x00ff);
	push_sp((xr & 0xff00) >> 8);
	push_sp(dp);
	push_sp(br);
	push_sp(ac);
	set_e_flag();
	push_sp(cc);
	set_i_flag();
	set_f_flag();
	pc = 0;
	pc = ((*read_8)(VECTOR_ILL_OPC)) << 8;
	pc |= (*read_8)(VECTOR_ILL_OPC+1);

	/*
	 * same as nmi number of cycles
	 */
	cycles += 19;
}

/*
 *  pc  dp ac br  xr   yr   us   sp  efhinzvc  N F I  NMI enabled/blocked
 * c000 00 01:ae 0000 d0d0 0000 0ffc -*-*---- 11 1 1  state normal/cwai/sync
 */
void mc6809::status(char *text_buffer)
{
	sprintf(text_buffer, " pc  dp ac br  xr   yr   us   sp  efhinzvc  N F I  NMI %s\n"
			"%04x %02x %02x:%02x "
			"%04x %04x %04x %04x "
			"%c%c%c%c%c%c%c%c "
			"%c%c %c %c  "
			"state normal",
			nmi_enabled ? "enabled" : "blocked",
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

void mc6809::stacks(char *text_buffer, int no)
{
	// display top of both stacks as 8 and 16 bit values
	text_buffer += sprintf(text_buffer, "  usp      ssp\n");
	for (int i=0; i<no; i++) {
		text_buffer += sprintf(text_buffer, "%04x %02x  %04x %02x",
			get_us() + i,
			read_8((uint16_t)(get_us() + i)),
			get_sp() + i,
			read_8((uint16_t)(get_sp() + i)));
		if (i < no-1) {
			text_buffer += sprintf(text_buffer, "\n");
		}
	}
}
