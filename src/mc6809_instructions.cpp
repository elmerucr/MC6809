/*
 * mc6809_opcodes.cpp  -  part of MC6809
 *
 * (C)2021-2022 elmerucr
 */

#include "mc6809.hpp"
#include <cstdio>

uint8_t  byte;
uint16_t word;
uint32_t dword;

/*
 * d_reg is a stand-in temporary variable to ease calculations
 * during individual instructions that deal with the d register
 */
uint16_t d_reg;

void mc6809::ill(uint16_t ea)
{
	// TODO !!!!!
	// "NEW": from 6309
	// push all registers, load vector illegal opcode ....
}

void mc6809::abx(uint16_t ea)
{
	xr += br;
}

void mc6809::adca(uint16_t ea)
{
	/*
	 * See: Osborne, A. 1976. An introduction to microcomputers
	 * Volume I Basic Concepts. SYBEX. pages 4-12 to 4-16.
	 */

	uint8_t old_carry = (is_c_flag_set() ? 1 : 0);

	byte = (*read_8)(ea);

	/*
	 * Half carry
	 */
	if (((ac & 0x0f) + (byte & 0x0f) + old_carry) & 0b00010000) {
		set_h_flag();
	} else {
		clear_h_flag();
	}

	bool bit_7_carry_in = (((ac & 0x7f) + (byte & 0x7f) + old_carry) & 0x80) ? true : false;

	word = ac + byte + old_carry;
	ac = word & 0x00ff;

	bool carry = (word & 0x0100) ? true : false;

	if (carry != bit_7_carry_in) set_v_flag(); else clear_v_flag();
	if (carry) set_c_flag(); else clear_c_flag();
	test_nz_flags(ac);
}

void mc6809::adcb(uint16_t ea)
{
	uint8_t old_carry = (is_c_flag_set() ? 1 : 0);

	byte = (*read_8)(ea);

	/*
	 * Half carry
	 */
	if (((br & 0x0f) + (byte & 0x0f) + old_carry) & 0b00010000) {
		set_h_flag();
	} else {
		clear_h_flag();
	}

	bool bit_7_carry_in = (((br & 0x7f) + (byte & 0x7f) + old_carry) & 0x80) ? true : false;

	word = br + byte + old_carry;
	br = word & 0x00ff;

	bool carry = (word & 0x0100) ? true : false;

	if (carry != bit_7_carry_in) set_v_flag(); else clear_v_flag();
	if (carry) set_c_flag(); else clear_c_flag();
	test_nz_flags(br);
}

void mc6809::adda(uint16_t ea)
{
	byte = (*read_8)(ea);

	/*
	 * Half carry
	 */
	if (((ac & 0x0f) + (byte & 0x0f)) & 0b00010000) {
		set_h_flag();
	} else {
		clear_h_flag();
	}

	bool bit_7_carry_in = (((ac & 0x7f) + (byte & 0x7f)) & 0x80) ? true : false;

	word = ac + byte;
	ac = word & 0x00ff;

	bool carry = (word & 0x0100) ? true : false;

	if (carry != bit_7_carry_in) set_v_flag(); else clear_v_flag();
	if (carry) set_c_flag(); else clear_c_flag();
	test_nz_flags(ac);
}

void mc6809::addb(uint16_t ea)
{
	byte = (*read_8)(ea);

	/*
	 * Half carry
	 */
	if (((br & 0x0f) + (byte & 0x0f)) & 0b00010000) {
		set_h_flag();
	} else {
		clear_h_flag();
	}

	bool bit_7_carry_in = (((br & 0x7f) + (byte & 0x7f)) & 0x80) ? true : false;

	word = br + byte;
	br = word & 0x00ff;

	bool carry = (word & 0x0100) ? true : false;

	if (carry != bit_7_carry_in) set_v_flag(); else clear_v_flag();
	if (carry) set_c_flag(); else clear_c_flag();
	test_nz_flags(br);
}

void mc6809::addd(uint16_t ea)
{
	word = ((*read_8)(ea++)) << 8;
	word |= (*read_8)(ea);
	
	d_reg = (ac << 8) | br;

	/* no need for half-carry here */

	bool bit_15_carry_in = (((d_reg & 0x7fff) + (word & 0x7fff)) & 0x8000) ? true : false;

	dword = d_reg + word;
	d_reg = dword & 0xffff;
	ac = (d_reg & 0xff00) >> 8;
	br = d_reg & 0xff;

	bool carry = (dword & 0x00010000) ? true : false;

	if(carry != bit_15_carry_in) set_v_flag(); else clear_v_flag();
	if(carry) set_c_flag(); else clear_c_flag();
	test_nz_flags_16(d_reg);
}

void mc6809::anda(uint16_t ea)
{
	byte = ac & (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(byte);
	ac = byte;
}

void mc6809::andb(uint16_t ea)
{
	byte = br & (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(byte);
	br = byte;
}

void mc6809::andcc(uint16_t ea)
{
	cc &= (*read_8)(ea);
}

void mc6809::asl(uint16_t ea)
{
	byte = (*read_8)(ea);

	if (byte & 0x80) set_c_flag(); else clear_c_flag();
	if (((byte & 0xc0) == 0x80) || ((byte & 0xc0) == 0x40))
		set_v_flag(); else clear_v_flag();

	byte <<= 1;

	test_nz_flags(byte);
	(*write_8)(ea, byte);
}

void mc6809::asla(uint16_t ea)
{
	if (ac & 0x80) set_c_flag(); else clear_c_flag();
	if (((ac & 0xc0) == 0x80) || ((ac & 0xc0) == 0x40))
		set_v_flag(); else clear_v_flag();

	ac <<= 1;

	test_nz_flags(ac);
}

void mc6809::aslb(uint16_t ea)
{
	if (br & 0x80) set_c_flag(); else clear_c_flag();
	if (((br & 0xc0) == 0x80) || ((br & 0xc0) == 0x40))
		set_v_flag(); else clear_v_flag();

	br <<= 1;

	test_nz_flags(br);
}

void mc6809::asr(uint16_t ea)
{
	byte = (*read_8)(ea);

	if (byte & 0x01) set_c_flag(); else clear_c_flag();
	bool bit7 = (byte & 0x80) ? true : false;

	byte >>= 1;
	if (bit7) byte |= 0x80; else byte &= 0x7f;

	test_nz_flags(byte);
	(*write_8)(ea, byte);
}

void mc6809::asra(uint16_t ea)
{
	if (ac & 0x01) set_c_flag(); else clear_c_flag();
	bool bit7 = (ac & 0x80) ? true : false;

	ac >>= 1;
	if (bit7) ac |= 0x80; else ac &= 0x7f;

	test_nz_flags(ac);
}

void mc6809::asrb(uint16_t ea)
{
	if (br & 0x01) set_c_flag(); else clear_c_flag();
	bool bit7 = (br & 0x80) ? true : false;

	br >>= 1;
	if (bit7) br |= 0x80; else br &= 0x7f;

	test_nz_flags(br);
}

void mc6809::beq(uint16_t ea)
{
	if (is_z_flag_set()) pc = ea;
}

void mc6809::bge(uint16_t ea)
{
	// both n and v set  OR  both n and v clear
	if ((is_n_flag_set() && is_v_flag_set()) || (is_n_flag_clear() && is_v_flag_clear())) {
		pc = ea;
	}
}

void mc6809::bgt(uint16_t ea)
{
	// (both n and v set  OR  both n and v clear)  AND  (z clear)
	if (((is_n_flag_set() && is_v_flag_set()) || (is_n_flag_clear() && is_v_flag_clear())) && is_z_flag_clear()) {
		pc = ea;
	}
}

void mc6809::bhi(uint16_t ea)
{
	if (is_z_flag_clear() && is_c_flag_clear()) {
		pc = ea;
	}
}

/*
 * bhs - Branch if Higher or Same
 * bcc - Branch if Carry Clear
 *
 * E.g. if no borrow was needed (carry clear) after a comparison, then
 * value in register must be higher than or the same as the compared value.
 */
void mc6809::bhs(uint16_t ea)
{
	if (is_c_flag_clear()) {
		pc = ea;
	}
}

void mc6809::bita(uint16_t ea)
{
	byte = ac & (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(byte);
}

void mc6809::bitb(uint16_t ea)
{
	byte = br & (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(byte);
}

void mc6809::ble(uint16_t ea)
{
	if (is_z_flag_set() || (is_n_flag_set() && is_v_flag_clear()) || (is_n_flag_clear() && is_v_flag_set())) {
		pc = ea;
	}
}

/*
 * blo - Branch if Lower
 * bcs - Branch if Carry Set
 *
 * E.g. if a borrow was needed (carry set) after a comparison, the
 * value in the register must be lower than the compared value.
 */
void mc6809::blo(uint16_t ea)
{
	if (is_c_flag_set()) {
		pc = ea;
	}
}

/*
 * bls - Branch if Lower or Same
 */
void mc6809::bls(uint16_t ea)
{
	if (is_c_flag_set() || is_z_flag_set()) {
		pc = ea;
	}
}

void mc6809::blt(uint16_t ea)
{
	if ((is_n_flag_set() && is_v_flag_clear()) || (is_n_flag_clear() && is_v_flag_set())) {
		pc = ea;
	}
}

void mc6809::bmi(uint16_t ea)
{
	if (is_n_flag_set()) {
		pc = ea;
	}
}

void mc6809::bne(uint16_t ea)
{
	if (is_z_flag_clear()) {
		pc = ea;
	}
}

void mc6809::bpl(uint16_t ea)
{
	if (is_n_flag_clear()) {
		pc = ea;
	}
}

void mc6809::bra(uint16_t ea)
{
	pc = ea;
}

void mc6809::brn(uint16_t ea)
{
	// does essentially nothing
}

void mc6809::bsr(uint16_t ea)
{
	push_sp(pc & 0x00ff);
	push_sp((pc & 0xff00) >> 8);
	pc = ea;
}

void mc6809::bvc(uint16_t ea)
{
	if (is_v_flag_clear()) {
		pc = ea;
	}
}

void mc6809::bvs(uint16_t ea)
{
	if (is_v_flag_set()) {
		pc = ea;
	}
}

void mc6809::clr(uint16_t ea)
{
	(*write_8)(ea, 0x00);
	clear_n_flag();
	set_z_flag();
	clear_v_flag();
	clear_c_flag();
}

void mc6809::clra(uint16_t ea)
{
	ac = 0x00;
	clear_n_flag();
	set_z_flag();
	clear_v_flag();
	clear_c_flag();
}

void mc6809::clrb(uint16_t ea)
{
	br = 0x00;
	clear_n_flag();
	set_z_flag();
	clear_v_flag();
	clear_c_flag();
}

void mc6809::cmpa(uint16_t ea)
{
	/* code inspired by virtualc64 */
	byte = (*read_8)(ea);
	word = ac - byte;

	if (word > 255) set_c_flag(); else clear_c_flag();

	if (((ac ^ word) & 0x80) && ((ac ^ byte) & 0x80)) set_v_flag(); else clear_v_flag();

	byte = word & 0xff;
	test_nz_flags(byte);
}

void mc6809::cmpb(uint16_t ea)
{
	/* code inspired by virtualc64 */
	byte = (*read_8)(ea);
	word = br - byte;

	if (word > 255) set_c_flag(); else clear_c_flag();

	if (((br ^ word) & 0x80) && ((br ^ byte) & 0x80)) set_v_flag(); else clear_v_flag();

	byte = word & 0xff;
	test_nz_flags(byte);
}

void mc6809::cmpd(uint16_t ea)
{
	/* code inspired by virtualc64 */
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	d_reg = (ac << 8) | br;
	dword = d_reg - word;

	if (dword > 65535) set_c_flag(); else clear_c_flag();

	if (((d_reg ^ dword) & 0x8000) && ((d_reg ^ word) & 0x8000)) set_v_flag(); else clear_v_flag();

	word = dword & 0xffff;
	test_nz_flags_16(word);
}

void mc6809::cmpu(uint16_t ea)
{
	/* code inspired by virtualc64 */
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	dword = us - word;

	if (dword > 65535) set_c_flag(); else clear_c_flag();

	if (((us ^ dword) & 0x8000) && ((us ^ word) & 0x8000)) set_v_flag(); else clear_v_flag();

	word = dword & 0xffff;
	test_nz_flags_16(word);
}

void mc6809::cmps(uint16_t ea)
{
	/* code inspired by virtualc64 */
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	dword = sp - word;

	if (dword > 65535) set_c_flag(); else clear_c_flag();

	if (((sp ^ dword) & 0x8000) && ((sp ^ word) & 0x8000)) set_v_flag(); else clear_v_flag();

	word = dword & 0xffff;
	test_nz_flags_16(word);
}

void mc6809::cmpx(uint16_t ea)
{
	/* code inspired by virtualc64 */
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	dword = xr - word;

	if (dword > 65535) set_c_flag(); else clear_c_flag();

	if (((xr ^ dword) & 0x8000) && ((xr ^ word) & 0x8000)) set_v_flag(); else clear_v_flag();

	word = dword & 0xffff;
	test_nz_flags_16(word);
}

void mc6809::cmpy(uint16_t ea)
{
	/* code inspired by virtualc64 */
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	dword = yr - word;

	if (dword > 65535) set_c_flag(); else clear_c_flag();

	if (((yr ^ dword) & 0x8000) && ((yr ^ word) & 0x8000)) set_v_flag(); else clear_v_flag();

	word = dword & 0xffff;
	test_nz_flags_16(word);
}

void mc6809::com(uint16_t ea)
{
	byte = (*read_8)(ea);
	byte = ~byte;
	(*write_8)(ea, byte);
	test_nz_flags(byte);
	clear_v_flag();
	set_c_flag();
}

void mc6809::coma(uint16_t ea)
{
	ac = ~ac;
	test_nz_flags(ac);
	clear_v_flag();
	set_c_flag();
}

void mc6809::comb(uint16_t ea)
{
	br = ~br;
	test_nz_flags(br);
	clear_v_flag();
	set_c_flag();
}

void mc6809::cwai(uint16_t ea)
{
	//
}

void mc6809::daa(uint16_t ea)
{
	if (is_h_flag_set() || ((ac & 0x0f) > 9))
		byte = 0x06; else byte = 0;
	if (is_c_flag_set() || (((ac & 0xf0) >> 4) > 9) ||
		((((ac & 0xf0) >> 4) > 8) && ((ac & 0x0f) > 9)))
		byte |= 0x60;
	word = ac + byte;
	ac = word & 0xff;
	if (word & 0x0100) set_c_flag(); else clear_c_flag();
	test_nz_flags(ac);
}

void mc6809::dec(uint16_t ea)
{
	byte = (*read_8)(ea);

	bool bit_7_carry_in = (((byte & 0x7f) + 0x7f) & 0x80) ? true : false;

	word = byte + 0xff;
	byte = word & 0x00ff;

	bool carry = (word & 0x0100) ? true : false;

	if (carry != bit_7_carry_in) set_v_flag(); else clear_v_flag();
	test_nz_flags(byte);

	(*write_8)(ea, byte);
}

void mc6809::deca(uint16_t ea)
{
	bool bit_7_carry_in = (((ac & 0x7f) + 0x7f) & 0x80) ? true : false;

	word = ac + 0xff;		// do an addition
	ac = word & 0x00ff;

	bool carry = (word & 0x0100) ? true : false;

	if (carry != bit_7_carry_in) set_v_flag(); else clear_v_flag();
	test_nz_flags(ac);
}

void mc6809::decb(uint16_t ea)
{
	bool bit_7_carry_in = (((br & 0x7f) + 0x7f) & 0x80) ? true : false;

	word = br + 0xff;		// do an addition
	br = word & 0x00ff;

	bool carry = (word & 0x0100) ? true : false;

	if (carry != bit_7_carry_in) set_v_flag(); else clear_v_flag();
	test_nz_flags(br);
}

void mc6809::eora(uint16_t ea)
{
	ac ^= (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(ac);
}

void mc6809::eorb(uint16_t ea)
{
	br ^= (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(br);
}

void mc6809::exg(uint16_t ea)
{
	/* illegal combinations do nothing */

	/* when the sp is written to, it enables nmi's */

	switch ((*read_8)(ea)) {
		/*
		 * exchange 16 bit registers
		 */
		//case 0b00000000: word = dr; dr = dr; dr = word; break;
		case 0b00000001: word = xr; xr = (ac << 8) | br; ac = (word & 0xff00) >> 8; br = word & 0xff; break;
		case 0b00000010: word = yr; yr = (ac << 8) | br; ac = (word & 0xff00) >> 8; br = word & 0xff; break;
		case 0b00000011: word = us; us = (ac << 8) | br; ac = (word & 0xff00) >> 8; br = word & 0xff; break;
		case 0b00000100: word = sp; sp = (ac << 8) | br; ac = (word & 0xff00) >> 8; br = word & 0xff; nmi_enabled = true; break;
		case 0b00000101: word = pc; pc = (ac << 8) | br; ac = (word & 0xff00) >> 8; br = word & 0xff; break;

		case 0b00010000: word = (ac << 8) | br; ac = (xr & 0xff00) >> 8; br = xr & 0xff; xr = word; break;
		//case 0b00010001: word = xr; xr = xr; xr = word; break;
		case 0b00010010: word = yr; yr = xr; xr = word; break;
		case 0b00010011: word = us; us = xr; xr = word; break;
		case 0b00010100: word = sp; sp = xr; xr = word; nmi_enabled = true; break;
		case 0b00010101: word = pc; pc = xr; xr = word; break;

		case 0b00100000: word = (ac << 8) | br; ac = (yr & 0xff00) >> 8; br = yr & 0xff; yr = word; break;
		case 0b00100001: word = xr; xr = yr; yr = word; break;
		//case 0b00100010: word = yr; yr = yr; yr = word; break;
		case 0b00100011: word = us; us = yr; yr = word; break;
		case 0b00100100: word = sp; sp = yr; yr = word; nmi_enabled = true; break;
		case 0b00100101: word = pc; pc = yr; yr = word; break;

		case 0b00110000: word = (ac << 8) | br; ac = (us & 0xff00) >> 8; br = us & 0xff; us = word; break;
		case 0b00110001: word = xr; xr = us; us = word; break;
		case 0b00110010: word = yr; yr = us; us = word; break;
		//case 0b00110011: word = us; us = us; us = word; break;
		case 0b00110100: word = sp; sp = us; us = word; nmi_enabled = true; break;
		case 0b00110101: word = pc; pc = us; us = word; break;

		case 0b01000000: word = (ac << 8) | br; ac = (sp & 0xff00) >> 8; br = sp & 0xff; sp = word; nmi_enabled = true; break;
		case 0b01000001: word = xr; xr = sp; sp = word; nmi_enabled = true; break;
		case 0b01000010: word = yr; yr = sp; sp = word; nmi_enabled = true; break;
		case 0b01000011: word = us; us = sp; sp = word; nmi_enabled = true; break;
		//case 0b01000100: word = sp; sp = sp; sp = word; nmi_blocked = false; break;
		case 0b01000101: word = pc; pc = sp; sp = word; nmi_enabled = true; break;

		case 0b01010000: word = (ac << 8) | br; ac = (pc & 0xff00) >> 8; br = pc & 0xff; pc = word; break;
		case 0b01010001: word = xr; xr = pc; pc = word; break;
		case 0b01010010: word = yr; yr = pc; pc = word; break;
		case 0b01010011: word = us; us = pc; pc = word; break;
		case 0b01010100: word = sp; sp = pc; pc = word; nmi_enabled = true; break;
		//case 0b01010101: word = pc; pc = pc; pc = word; break;

		/*
		 * exchange 8 bit registers
		 */
		//case 0b10001000: byte = ac; ac = ac; ac = byte; break;
		case 0b10001001: byte = br; br = ac; ac = byte; break;
		case 0b10001010: byte = cc; cc = ac; ac = byte; break;
		case 0b10001011: byte = dp; dp = ac; ac = byte; break;

		case 0b10011000: byte = ac; ac = br; br = byte; break;
		//case 0b10011001: byte = br; br = br; br = byte; break;
		case 0b10011010: byte = cc; cc = br; br = byte; break;
		case 0b10011011: byte = dp; dp = br; br = byte; break;

		case 0b10101000: byte = ac; ac = cc; cc = byte; break;
		case 0b10101001: byte = br; br = cc; cc = byte; break;
		//case 0b10101010: byte = cc; cc = cc; cc = byte; break;
		case 0b10101011: byte = dp; dp = cc; cc = byte; break;

		case 0b10111000: byte = ac; ac = dp; dp = byte; break;
		case 0b10111001: byte = br; br = dp; dp = byte; break;
		case 0b10111010: byte = cc; cc = dp; dp = byte; break;
		//case 0b10111011: byte = dp; dp = dp; dp = byte; break;

		default:
			// do nothing
			break;
	}
}

void mc6809::inc(uint16_t ea)
{
	byte = (*read_8)(ea);

	bool bit_7_carry_in = (((byte & 0x7f) + 0x01) & 0x80) ? true : false;

	word = byte + 0x01;
	byte = word & 0x00ff;

	bool carry = (word & 0x0100) ? true : false;

	if (carry != bit_7_carry_in) set_v_flag(); else clear_v_flag();
	test_nz_flags(byte);

	(*write_8)(ea, byte);
}

void mc6809::inca(uint16_t ea)
{
	bool bit_7_carry_in = (((ac & 0x7f) + 0x01) & 0x80) ? true : false;

	word = ac + 0x01;		// do an addition
	ac = word & 0x00ff;

	bool carry = (word & 0x0100) ? true : false;

	if (carry != bit_7_carry_in) set_v_flag(); else clear_v_flag();
	test_nz_flags(ac);
}

void mc6809::incb(uint16_t ea)
{
	bool bit_7_carry_in = (((br & 0x7f) + 0x01) & 0x80) ? true : false;

	word = br + 0x01;		// do an addition
	br = word & 0x00ff;

	bool carry = (word & 0x0100) ? true : false;

	if (carry != bit_7_carry_in) set_v_flag(); else clear_v_flag();
	test_nz_flags(br);
}

void mc6809::jmp(uint16_t ea)
{
	pc = ea;
}

void mc6809::jsr(uint16_t ea)
{
	push_sp(pc & 0x00ff);
	push_sp((pc & 0xff00) >> 8);
	pc = ea;
}

void mc6809::lbeq(uint16_t ea)
{
	if (is_z_flag_set()) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lbge(uint16_t ea)
{
	// both n and v set  OR  both n and v clear
	if ((is_n_flag_set() && is_v_flag_set()) || (is_n_flag_clear() && is_v_flag_clear())) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lbgt(uint16_t ea)
{
	// (both n and v set  OR  both n and v clear)  AND  (z clear)
	if (((is_n_flag_set() && is_v_flag_set()) || (is_n_flag_clear() && is_v_flag_clear())) && is_z_flag_clear()) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lbhi(uint16_t ea)
{
	if (is_z_flag_clear() && is_c_flag_clear()) {
		pc = ea;
		cycles += 1;
	}
}

/*
 * lbhs - Branch if Higher or Same
 * lbcc - Branch if Carry Clear
 *
 * E.g. if no borrow was needed (carry clear) after a comparison, then
 * value in register must be higher or the same as the compared value.
 */
void mc6809::lbhs(uint16_t ea)
{
	if (is_c_flag_clear()) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lble(uint16_t ea)
{
	if (is_z_flag_set() || (is_n_flag_set() && is_v_flag_clear()) || (is_n_flag_clear() && is_v_flag_set())) {
		pc = ea;
		cycles += 1;
	}
}

/*
 * lblo - Branch if Lower
 * lbcs - Branch if Carry Set
 *
 * E.g. if a borrow was needed (carry set) after a comparison, the
 * value in the register must be lower than the compared value.
 */
void mc6809::lblo(uint16_t ea)
{
	if (is_c_flag_set()) {
		pc = ea;
		cycles += 1;
	}
}

/*
 * bls - Branch if Lower or Same
 */
void mc6809::lbls(uint16_t ea)
{
	if (is_c_flag_set() || is_z_flag_set()) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lblt(uint16_t ea)
{
	if ((is_n_flag_set() && is_v_flag_clear()) || (is_n_flag_clear() && is_v_flag_set())) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lbmi(uint16_t ea)
{
	if (is_n_flag_set()) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lbne(uint16_t ea)
{
	if (is_z_flag_clear()) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lbpl(uint16_t ea)
{
	if (is_n_flag_clear()) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lbra(uint16_t ea)
{
	pc = ea;
}

void mc6809::lbrn(uint16_t ea)
{
	// does essentially nothing
}

void mc6809::lbsr(uint16_t ea)
{
	push_sp(pc & 0x00ff);
	push_sp((pc & 0xff00) >> 8);
	pc = ea;
}

void mc6809::lbvc(uint16_t ea)
{
	if (is_v_flag_clear()) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lbvs(uint16_t ea)
{
	if (is_v_flag_set()) {
		pc = ea;
		cycles += 1;
	}
}

void mc6809::lda(uint16_t ea)
{
	ac = (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(ac);
}

void mc6809::ldb(uint16_t ea)
{
	br = (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(br);
}

void mc6809::ldd(uint16_t ea)
{
	ac = (*read_8)(ea++);
	br = (*read_8)((uint16_t)ea);
	d_reg = (ac << 8) | br;
	clear_v_flag();
	test_nz_flags_16(d_reg);
}

void mc6809::lds(uint16_t ea)
{
	sp = (*read_8)(ea++) << 8;
	sp |= (*read_8)((uint16_t)ea);
	clear_v_flag();
	test_nz_flags_16(sp);

	// a write to system stackpointer enables nmi's
	nmi_enabled = true;
}

void mc6809::ldu(uint16_t ea)
{
	us = (*read_8)(ea++) << 8;
	us |= (*read_8)((uint16_t)ea);
	clear_v_flag();
	test_nz_flags_16(us);
}

void mc6809::ldx(uint16_t ea)
{
	xr = (*read_8)(ea++) << 8;
	xr |= (*read_8)((uint16_t)ea);
	clear_v_flag();
	test_nz_flags_16(xr);
}

void mc6809::ldy(uint16_t ea)
{
	yr = (*read_8)(ea++) << 8;
	yr |= (*read_8)((uint16_t)ea);
	clear_v_flag();
	test_nz_flags_16(yr);
}

void mc6809::leax(uint16_t ea)
{
	test_z_flag_16(ea);
	xr = ea;
}

void mc6809::leay(uint16_t ea)
{
	test_z_flag_16(ea);
	yr = ea;
}

void mc6809::leas(uint16_t ea)
{
	test_z_flag_16(ea);
	sp = ea;

	// a write to system stackpointer enables nmi's
	nmi_enabled = true;
}

void mc6809::leau(uint16_t ea)
{
	test_z_flag_16(ea);
	us = ea;
}

void mc6809::lsr(uint16_t ea)
{
	byte = (*read_8)(ea);
	if (byte & 0x01) set_c_flag(); else clear_c_flag();
	byte >>= 1;
	test_z_flag(byte);
	clear_n_flag();
	(*write_8)(ea, byte);
}

void mc6809::lsra(uint16_t ea)
{
	if (ac & 0x01) set_c_flag(); else clear_c_flag();
	ac >>= 1;
	test_z_flag(ac);
	clear_n_flag();
}

void mc6809::lsrb(uint16_t ea)
{
	if (br & 0x01) set_c_flag(); else clear_c_flag();
	br >>= 1;
	test_z_flag(br);
	clear_n_flag();
}

void mc6809::mul(uint16_t ea)
{
	d_reg = ac * br;
	test_z_flag_16(d_reg);
	ac = (d_reg & 0xff00) >> 8;
	br = d_reg & 0xff;
	if (br & 0x80) set_c_flag(); else clear_c_flag();
}

void mc6809::neg(uint16_t ea)
{
	byte = (*read_8)(ea);
	if (byte == 0x80) set_v_flag(); else clear_v_flag();
	if (byte == 0x00) clear_c_flag(); else set_c_flag();
	byte = ~byte;
	byte++;
	test_nz_flags(byte);
	(*write_8)(ea, byte);
}

void mc6809::nega(uint16_t ea)
{
	if (ac == 0x80) set_v_flag(); else clear_v_flag();
	if (ac == 0x00) clear_c_flag(); else set_c_flag();
	ac = ~ac;
	ac++;
	test_nz_flags(ac);
}

void mc6809::negb(uint16_t ea)
{
	if (br == 0x80) set_v_flag(); else clear_v_flag();
	if (br == 0x00) clear_c_flag(); else set_c_flag();
	br = ~br;
	br++;
	test_nz_flags(br);
}

void mc6809::nop(uint16_t ea)
{
	// does nothing
}

void mc6809::ora(uint16_t ea)
{
	byte = ac | (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(byte);
	ac = byte;
}

void mc6809::orb(uint16_t ea)
{
	byte = br | (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(byte);
	br = byte;
}

void mc6809::orcc(uint16_t ea)
{
	cc |= (*read_8)(ea);
}

void mc6809::page2(uint16_t ea)
{
	uint8_t opcode = (*read_8)(pc++);
	cycles += cycles_page2[opcode];

	bool am_legal;

	uint16_t effective_address = (this->*addressing_modes_page2[opcode])(&am_legal);
	(this->*opcodes_page2[opcode])(effective_address);
}

void mc6809::page3(uint16_t ea)
{
	uint8_t opcode = (*read_8)(pc++);
	cycles += cycles_page3[opcode];

	bool am_legal;

	uint16_t effective_address = (this->*addressing_modes_page3[opcode])(&am_legal);
	(this->*opcodes_page3[opcode])(effective_address);
}

void mc6809::pshs(uint16_t ea)
{
	byte = (*read_8)(ea);

	if (byte & 0x80) { push_sp(pc & 0x00ff); push_sp((pc & 0xff00) >> 8); cycles += 2; }
	if (byte & 0x40) { push_sp(us & 0x00ff); push_sp((us & 0xff00) >> 8); cycles += 2; }
	if (byte & 0x20) { push_sp(yr & 0x00ff); push_sp((yr & 0xff00) >> 8); cycles += 2; }
	if (byte & 0x10) { push_sp(xr & 0x00ff); push_sp((xr & 0xff00) >> 8); cycles += 2; }
	if (byte & 0x08) { push_sp(dp);                                       cycles += 1; }
	if (byte & 0x04) { push_sp(br);                                       cycles += 1; }
	if (byte & 0x02) { push_sp(ac);                                       cycles += 1; }
	if (byte & 0x01) { push_sp(cc);                                       cycles += 1; }
}

void mc6809::pshu(uint16_t ea)
{
	byte = (*read_8)(ea);

	if (byte & 0x80) { push_us(pc & 0x00ff); push_us((pc & 0xff00) >> 8); cycles += 2; }
	if (byte & 0x40) { push_us(sp & 0x00ff); push_us((sp & 0xff00) >> 8); cycles += 2; }
	if (byte & 0x20) { push_us(yr & 0x00ff); push_us((yr & 0xff00) >> 8); cycles += 2; }
	if (byte & 0x10) { push_us(xr & 0x00ff); push_us((xr & 0xff00) >> 8); cycles += 2; }
	if (byte & 0x08) { push_us(dp);                                       cycles += 1; }
	if (byte & 0x04) { push_us(br);                                       cycles += 1; }
	if (byte & 0x02) { push_us(ac);                                       cycles += 1; }
	if (byte & 0x01) { push_us(cc);                                       cycles += 1; }
}

void mc6809::puls(uint16_t ea)
{
	byte = (*read_8)(ea);

	if (byte & 0x01) { cc   = pull_sp();                                    cycles += 1; }
	if (byte & 0x02) { ac   = pull_sp();                                    cycles += 1; }
	if (byte & 0x04) { br   = pull_sp();                                    cycles += 1; }
	if (byte & 0x08) { dp   = pull_sp();                                    cycles += 1; }
	if (byte & 0x10) { word = pull_sp() << 8; word |= pull_sp(); xr = word; cycles += 2; }
	if (byte & 0x20) { word = pull_sp() << 8; word |= pull_sp(); yr = word; cycles += 2; }
	if (byte & 0x40) { word = pull_sp() << 8; word |= pull_sp(); us = word; cycles += 2; }
	if (byte & 0x80) { word = pull_sp() << 8; word |= pull_sp(); pc = word; cycles += 2; }
}

void mc6809::pulu(uint16_t ea)
{
	byte = (*read_8)(ea);

	if (byte & 0x01) { cc   = pull_us();                                    cycles += 1; }
	if (byte & 0x02) { ac   = pull_us();                                    cycles += 1; }
	if (byte & 0x04) { br   = pull_us();                                    cycles += 1; }
	if (byte & 0x08) { dp   = pull_us();                                    cycles += 1; }
	if (byte & 0x10) { word = pull_us() << 8; word |= pull_us(); xr = word; cycles += 2; }
	if (byte & 0x20) { word = pull_us() << 8; word |= pull_us(); yr = word; cycles += 2; }
	if (byte & 0x40) { word = pull_us() << 8; word |= pull_us(); sp = word; cycles += 2; }
	if (byte & 0x80) { word = pull_us() << 8; word |= pull_us(); pc = word; cycles += 2; }
}

void mc6809::rol(uint16_t ea)
{
	byte = (*read_8)(ea);
	uint8_t old_carry = cc & C_FLAG;
	if (((byte & 0b11000000) == 0b01000000) || ((byte & 0b11000000) == 0b10000000))
		set_v_flag(); else clear_v_flag();
	if (byte & 0x80) set_c_flag(); else clear_c_flag();
	byte <<= 1;
	byte |= old_carry;
	test_nz_flags(byte);
	(*write_8)(ea, byte);
}

void mc6809::rola(uint16_t ea)
{
	uint8_t old_carry = cc & C_FLAG;
	if (((ac & 0b11000000) == 0b01000000) || ((ac & 0b11000000) == 0b10000000))
		set_v_flag(); else clear_v_flag();
	if (ac & 0x80) set_c_flag(); else clear_c_flag();
	ac <<= 1;
	ac |= old_carry;
	test_nz_flags(ac);
}

void mc6809::rolb(uint16_t ea)
{
	uint8_t old_carry = cc & C_FLAG;
	if (((br & 0b11000000) == 0b01000000) || ((br & 0b11000000) == 0b10000000))
		set_v_flag(); else clear_v_flag();
	if (br & 0x80) set_c_flag(); else clear_c_flag();
	br <<= 1;
	br |= old_carry;
	test_nz_flags(br);
}

void mc6809::ror(uint16_t ea)
{
	byte = (*read_8)(ea);
	bool old_carry = is_c_flag_set();
	if (byte & 0x01) set_c_flag(); else clear_c_flag();
	byte >>= 1;
	if (old_carry) byte |= 0x80;
	test_nz_flags(byte);
	(*write_8)(ea, byte);
}

void mc6809::rora(uint16_t ea)
{
	bool old_carry = is_c_flag_set();
	if (ac & 0x01) set_c_flag(); else clear_c_flag();
	ac >>= 1;
	if (old_carry) ac |= 0x80;
	test_nz_flags(ac);
}

void mc6809::rorb(uint16_t ea)
{
	bool old_carry = is_c_flag_set();
	if (br & 0x01) set_c_flag(); else clear_c_flag();
	br >>= 1;
	if (old_carry) br |= 0x80;
	test_nz_flags(br);
}

void mc6809::rti(uint16_t ea)
{
	cc = pull_sp();
	if (is_e_flag_set()) {
		ac = pull_sp();
		br = pull_sp();
		dp = pull_sp();
		word = pull_sp() << 8;
		word |= pull_sp();
		xr = word;
		word = pull_sp() << 8;
		word |= pull_sp();
		yr = word;
		word = pull_sp() << 8;
		word |= pull_sp();
		us = word;

		cycles += 9;
	}
	word = pull_sp() << 8;
	word |= pull_sp();
	pc = word;
}

void mc6809::rts(uint16_t ea)
{
	word = pull_sp() << 8;
	word |= pull_sp();
	pc = word;
}

void mc6809::sbca(uint16_t ea)
{
	/* code inspired by virtualc64 */
	byte = (*read_8)(ea);
	word = ac - byte - (is_c_flag_set() ? 1 : 0);

	if (word > 255) set_c_flag(); else clear_c_flag();

	if (((ac ^ word) & 0x80) && ((ac ^ byte) & 0x80)) set_v_flag(); else clear_v_flag();

	ac = word & 0xff;
	test_nz_flags(ac);
}

void mc6809::sbcb(uint16_t ea)
{
	/* code inspired by virtualc64 */
	byte = (*read_8)(ea);
	word = br - byte - (is_c_flag_set() ? 1 : 0);

	if (word > 255) set_c_flag(); else clear_c_flag();

	if (((br ^ word) & 0x80) && ((br ^ byte) & 0x80)) set_v_flag(); else clear_v_flag();

	br = word & 0xff;
	test_nz_flags(br);
}

void mc6809::sex(uint16_t ea)
{
	if (br & 0x80) ac = 0xff; else ac = 0x00;
	test_nz_flags(br);
}

void mc6809::sta(uint16_t ea)
{
	(*write_8)(ea, ac);
	clear_v_flag();
	test_nz_flags(ac);
}

void mc6809::stb(uint16_t ea)
{
	(*write_8)(ea, br);
	clear_v_flag();
	test_nz_flags(br);
}

void mc6809::std(uint16_t ea)
{
	(*write_8)(ea++, ac);
	(*write_8)(ea, br);
	d_reg = (ac << 8) | br;
	clear_v_flag();
	test_nz_flags_16(d_reg);
}

void mc6809::stu(uint16_t ea)
{
	(*write_8)(ea++, us >> 8);
	(*write_8)(ea, us & 0xff);
	clear_v_flag();
	test_nz_flags_16(us);
}

void mc6809::sts(uint16_t ea)
{
	(*write_8)(ea++, sp >> 8);
	(*write_8)(ea, sp & 0xff);
	clear_v_flag();
	test_nz_flags_16(sp);
}

void mc6809::stx(uint16_t ea)
{
	(*write_8)(ea++, xr >> 8);
	(*write_8)(ea, xr & 0xff);
	clear_v_flag();
	test_nz_flags_16(xr);
}

void mc6809::sty(uint16_t ea)
{
	(*write_8)(ea++, yr >> 8);
	(*write_8)(ea, yr & 0xff);
	clear_v_flag();
	test_nz_flags_16(yr);
}

void mc6809::suba(uint16_t ea)
{
	/* code inspired by virtualc64 */
	byte = (*read_8)(ea);
	word = ac - byte;

	if (word > 255) set_c_flag(); else clear_c_flag();

	if (((ac ^ word) & 0x80) && ((ac ^ byte) & 0x80)) set_v_flag(); else clear_v_flag();

	ac = word & 0xff;
	test_nz_flags(ac);
}

void mc6809::subb(uint16_t ea)
{
	/* code inspired by virtualc64 */
	byte = (*read_8)(ea);
	word = br - byte;

	if (word > 255) set_c_flag(); else clear_c_flag();

	if (((br ^ word) & 0x80) && ((br ^ byte) & 0x80)) set_v_flag(); else clear_v_flag();

	br = word & 0xff;
	test_nz_flags(br);
}

void mc6809::subd(uint16_t ea)
{
	/* code inspired by virtualc64 */
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	
	d_reg = (ac << 8) | br;

	dword = d_reg - word;

	if (dword > 65535) set_c_flag(); else clear_c_flag();

	if (((d_reg ^ dword) & 0x8000) && ((d_reg ^ word) & 0x8000)) set_v_flag(); else clear_v_flag();

	d_reg = dword & 0xffff;
	ac = (d_reg & 0xff00) >> 8;
	br = d_reg & 0xff;
	test_nz_flags_16(d_reg);
}

void mc6809::swi(uint16_t ea)
{
	set_e_flag();
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
	push_sp(cc);
	set_i_flag();
	set_f_flag();
	pc = 0;
	pc = ((*read_8)(VECTOR_SWI)) << 8;
	pc |= (*read_8)(VECTOR_SWI+1);
}

void mc6809::swi2(uint16_t ea)
{
	set_e_flag();
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
	push_sp(cc);
	pc = 0;
	pc = ((*read_8)(VECTOR_SWI2)) << 8;
	pc |= (*read_8)(VECTOR_SWI2+1);
}

void mc6809::swi3(uint16_t ea)
{
	set_e_flag();
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
	push_sp(cc);
	pc = 0;
	pc = ((*read_8)(VECTOR_SWI3)) << 8;
	pc |= (*read_8)(VECTOR_SWI3+1);
}

void mc6809::sync(uint16_t ea)
{
	//
}

void mc6809::tfr(uint16_t ea)
{
	/* illegal combinations do nothing */

	/* when sp is written to, nmi's are enabled */

	switch ((*read_8)(ea)) {
		/*
		 * transfer 16 bit registers
		 */
		//case 0b00000000: dr = (ac << 8) | br; break;
		case 0b00000001: xr = (ac << 8) | br; break;
		case 0b00000010: yr = (ac << 8) | br; break;
		case 0b00000011: us = (ac << 8) | br; break;
		case 0b00000100: sp = (ac << 8) | br; nmi_enabled = true; break;
		case 0b00000101: pc = (ac << 8) | br; break;

		case 0b00010000: ac = (xr & 0xff00) >> 8; br = xr & 0xff; break;
		//case 0b00010001: xr = xr; break;
		case 0b00010010: yr = xr; break;
		case 0b00010011: us = xr; break;
		case 0b00010100: sp = xr; nmi_enabled = true; break;
		case 0b00010101: pc = xr; break;

		case 0b00100000: ac = (yr & 0xff00) >> 8; br = yr & 0xff; break;
		case 0b00100001: xr = yr; break;
		//case 0b00100010: yr = yr; break;
		case 0b00100011: us = yr; break;
		case 0b00100100: sp = yr; nmi_enabled = true; break;
		case 0b00100101: pc = yr; break;

		case 0b00110000: ac = (us & 0xff00) >> 8; br = us & 0xff; break;
		case 0b00110001: xr = us; break;
		case 0b00110010: yr = us; break;
		//case 0b00110011: us = us; break;
		case 0b00110100: sp = us; nmi_enabled = true; break;
		case 0b00110101: pc = us; break;

		case 0b01000000: ac = (sp & 0xff00) >> 8; br = sp & 0xff; break;
		case 0b01000001: xr = sp; break;
		case 0b01000010: yr = sp; break;
		case 0b01000011: us = sp; break;
		//case 0b01000100: sp = sp; nmi_blocked = false; break;
		case 0b01000101: pc = sp; break;

		case 0b01010000: ac = (pc & 0xff00) >> 8; br = pc & 0xff; break;
		case 0b01010001: xr = pc; break;
		case 0b01010010: yr = pc; break;
		case 0b01010011: us = pc; break;
		case 0b01010100: sp = pc; nmi_enabled = true; break;
		//case 0b01010101: pc = pc; break;

		/*
		 * transfer 8 bit registers
		 */
		//case 0b10001000: ac = ac; break;
		case 0b10001001: br = ac; break;
		case 0b10001010: cc = ac; break;
		case 0b10001011: dp = ac; break;

		case 0b10011000: ac = br; break;
		//case 0b10011001: br = br; break;
		case 0b10011010: cc = br; break;
		case 0b10011011: dp = br; break;

		case 0b10101000: ac = cc; break;
		case 0b10101001: br = cc; break;
		//case 0b10101010: cc = cc; break;
		case 0b10101011: dp = cc; break;

		case 0b10111000: ac = dp; break;
		case 0b10111001: br = dp; break;
		case 0b10111010: cc = dp; break;
		//case 0b10111011: dp = dp; break;

		default:
			// do nothing
			break;
	}
}

void mc6809::tst(uint16_t ea)
{
	test_nz_flags((*read_8)(ea));
	clear_v_flag();
}

void mc6809::tsta(uint16_t ea)
{
	test_nz_flags(ac);
	clear_v_flag();
}

void mc6809::tstb(uint16_t ea)
{
	test_nz_flags(br);
	clear_v_flag();
}
