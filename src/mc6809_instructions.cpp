/*
 * mc6809_opcodes.cpp  -  part of MC6809
 *
 * (c)2021 elmerucr
 */

#include "mc6809.hpp"
#include <cstdio>

uint8_t byte;
uint16_t word;

void mc6809::ill(uint16_t ea)
{
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

	/* no need for half-carry here */

	bool bit_15_carry_in = (((dr & 0x7fff) + (word & 0x7fff)) & 0x8000) ? true : false;

	uint32_t dword = dr + word;
	dr = dword & 0xffff;

	bool carry = (dword & 0x00010000) ? true : false;

	if(carry != bit_15_carry_in) set_v_flag(); else clear_v_flag();
	if(carry) set_c_flag(); else clear_c_flag();
	test_nz_flags_16(dr);
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
	//
}

void mc6809::asla(uint16_t ea)
{
	//
}

void mc6809::aslb(uint16_t ea)
{
	//
}

void mc6809::asr(uint16_t ea)
{
	//
}

void mc6809::asra(uint16_t ea)
{
	//
}

void mc6809::asrb(uint16_t ea)
{
	//
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

// bcc
void mc6809::bhs(uint16_t ea)
{
	if (is_c_flag_clear()) {
		pc = ea;
	}
}

void mc6809::bita(uint16_t ea)
{
	//
}

void mc6809::bitb(uint16_t ea)
{
	//
}

void mc6809::ble(uint16_t ea)
{
	if (is_z_flag_set() || (is_n_flag_set() && is_v_flag_clear()) || (is_n_flag_clear() && is_v_flag_set())) {
		pc = ea;
	}
}

// bcs
void mc6809::blo(uint16_t ea)
{
	if (is_c_flag_set()) {
		pc = ea;
	}
}

// CHECK
//
void mc6809::bls(uint16_t ea)
{
	if ((is_c_flag_set() && is_z_flag_clear()) || (is_c_flag_clear() && is_z_flag_set())) {
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
	//
}

void mc6809::cmpb(uint16_t ea)
{
	//
}

void mc6809::cmpd(uint16_t ea)
{
	//
}

void mc6809::cmpu(uint16_t ea)
{
	//
}

void mc6809::cmps(uint16_t ea)
{
	//
}

void mc6809::cmpx(uint16_t ea)
{
	//
}

void mc6809::cmpy(uint16_t ea)
{
	//
}

void mc6809::com(uint16_t ea)
{
	//
}

void mc6809::coma(uint16_t ea)
{
	//
}

void mc6809::comb(uint16_t ea)
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

void mc6809::deca(uint16_t ea)
{
	//
}

void mc6809::decb(uint16_t ea)
{
	//
}

void mc6809::eora(uint16_t ea)
{
	byte = ac ^ (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(byte);
	ac = byte;
}

void mc6809::eorb(uint16_t ea)
{
	byte = br ^ (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(byte);
	br = byte;
}

void mc6809::exg(uint16_t ea)
{
	/* illegal combinations do nothing */

	switch ((*read_8)(ea)) {
		/*
		 * exchange 16 bit registers
		 */
		//case 0b00000000: word = dr; dr = dr; dr = word; break;
		case 0b00000001: word = xr; xr = dr; dr = word; break;
		case 0b00000010: word = yr; yr = dr; dr = word; break;
		case 0b00000011: word = us; us = dr; dr = word; break;
		case 0b00000100: word = sp; sp = dr; dr = word; break;
		case 0b00000101: word = pc; pc = dr; dr = word; break;

		case 0b00010000: word = dr; dr = xr; xr = word; break;
		//case 0b00010001: word = xr; xr = xr; xr = word; break;
		case 0b00010010: word = yr; yr = xr; xr = word; break;
		case 0b00010011: word = us; us = xr; xr = word; break;
		case 0b00010100: word = sp; sp = xr; xr = word; break;
		case 0b00010101: word = pc; pc = xr; xr = word; break;

		case 0b00100000: word = dr; dr = yr; yr = word; break;
		case 0b00100001: word = xr; xr = yr; yr = word; break;
		//case 0b00100010: word = yr; yr = yr; yr = word; break;
		case 0b00100011: word = us; us = yr; yr = word; break;
		case 0b00100100: word = sp; sp = yr; yr = word; break;
		case 0b00100101: word = pc; pc = yr; yr = word; break;

		case 0b00110000: word = dr; dr = us; us = word; break;
		case 0b00110001: word = xr; xr = us; us = word; break;
		case 0b00110010: word = yr; yr = us; us = word; break;
		//case 0b00110011: word = us; us = us; us = word; break;
		case 0b00110100: word = sp; sp = us; us = word; break;
		case 0b00110101: word = pc; pc = us; us = word; break;

		case 0b01000000: word = dr; dr = sp; sp = word; break;
		case 0b01000001: word = xr; xr = sp; sp = word; break;
		case 0b01000010: word = yr; yr = sp; sp = word; break;
		case 0b01000011: word = us; us = sp; sp = word; break;
		//case 0b01000100: word = sp; sp = sp; sp = word; break;
		case 0b01000101: word = pc; pc = sp; sp = word; break;

		case 0b01010000: word = dr; dr = pc; pc = word; break;
		case 0b01010001: word = xr; xr = pc; pc = word; break;
		case 0b01010010: word = yr; yr = pc; pc = word; break;
		case 0b01010011: word = us; us = pc; pc = word; break;
		case 0b01010100: word = sp; sp = pc; pc = word; break;
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
	//
}

void mc6809::inca(uint16_t ea)
{
	//
}

void mc6809::incb(uint16_t ea)
{
	//
}

void mc6809::jmp(uint16_t ea)
{
	//
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

void mc6809::lblo(uint16_t ea)
{
	if (is_c_flag_set()) {
		pc = ea;
		cycles += 1;
	}
}

// CHECK
//
void mc6809::lbls(uint16_t ea)
{
	if ((is_c_flag_set() && is_z_flag_clear()) || (is_c_flag_clear() && is_z_flag_set())) {
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
	byte = (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(byte);
	ac = byte;
}

void mc6809::ldb(uint16_t ea)
{
	byte = (*read_8)(ea);
	clear_v_flag();
	test_nz_flags(byte);
	br = byte;
}

void mc6809::ldd(uint16_t ea)
{
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	clear_v_flag();
	test_nz_flags_16(word);
	dr = word;
}

void mc6809::lds(uint16_t ea)
{
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	clear_v_flag();
	test_nz_flags_16(word);
	sp = word;

	// a write to system stackpointer enables nmi's
	nmi_blocked = false;
}

void mc6809::ldu(uint16_t ea)
{
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	clear_v_flag();
	test_nz_flags_16(word);
	us = word;
}

void mc6809::ldx(uint16_t ea)
{
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	clear_v_flag();
	test_nz_flags_16(word);
	xr = word;
}

void mc6809::ldy(uint16_t ea)
{
	word = (*read_8)(ea++) << 8;
	word |= (*read_8)((uint16_t)ea);
	clear_v_flag();
	test_nz_flags_16(word);
	yr = word;
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
	nmi_blocked = false;
}

void mc6809::leau(uint16_t ea)
{
	test_z_flag_16(ea);
	us = ea;
}

void mc6809::lsr(uint16_t ea)
{
	//
}

void mc6809::lsra(uint16_t ea)
{
	//
}

void mc6809::lsrb(uint16_t ea)
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

void mc6809::nega(uint16_t ea)
{
	//
}

void mc6809::negb(uint16_t ea)
{
	//
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
	//
}

void mc6809::rola(uint16_t ea)
{
	//
}

void mc6809::rolb(uint16_t ea)
{
	//
}

void mc6809::ror(uint16_t ea)
{
	//
}

void mc6809::rora(uint16_t ea)
{
	//
}

void mc6809::rorb(uint16_t ea)
{
	//
}

void mc6809::rti(uint16_t ea)
{
	//
}

void mc6809::rts(uint16_t ea)
{
	word = pull_sp() << 8;
	word |= pull_sp();
	pc = word;
}

void mc6809::sbca(uint16_t ea)
{
	//
}

void mc6809::sbcb(uint16_t ea)
{
	//
}

void mc6809::sex(uint16_t ea)
{
	//
}

void mc6809::sta(uint16_t ea)
{
	//
}

void mc6809::stb(uint16_t ea)
{
	//
}

void mc6809::std(uint16_t ea)
{
	//
}

void mc6809::stu(uint16_t ea)
{
	//
}

void mc6809::sts(uint16_t ea)
{
	//
}

void mc6809::stx(uint16_t ea)
{
	//
}

void mc6809::sty(uint16_t ea)
{
	//
}

void mc6809::suba(uint16_t ea)
{
	//
}

void mc6809::subb(uint16_t ea)
{
	//
}

void mc6809::subd(uint16_t ea)
{
	//
}

void mc6809::swi(uint16_t ea)
{
	//
}

void mc6809::swi2(uint16_t ea)
{
	printf("swi2() not implemented\n");
}

void mc6809::swi3(uint16_t ea)
{
	printf("swi3() not implemented\n");
}

void mc6809::sync(uint16_t ea)
{
	//
}

void mc6809::tfr(uint16_t ea)
{
	/* illegal combinations do nothing */

	switch ((*read_8)(ea)) {
		/*
		 * transfer 16 bit registers
		 */
		//case 0b00000000: dr = dr; break;
		case 0b00000001: xr = dr; break;
		case 0b00000010: yr = dr; break;
		case 0b00000011: us = dr; break;
		case 0b00000100: sp = dr; break;
		case 0b00000101: pc = dr; break;

		case 0b00010000: dr = xr; break;
		//case 0b00010001: xr = xr; break;
		case 0b00010010: yr = xr; break;
		case 0b00010011: us = xr; break;
		case 0b00010100: sp = xr; break;
		case 0b00010101: pc = xr; break;

		case 0b00100000: dr = yr; break;
		case 0b00100001: xr = yr; break;
		//case 0b00100010: yr = yr; break;
		case 0b00100011: us = yr; break;
		case 0b00100100: sp = yr; break;
		case 0b00100101: pc = yr; break;

		case 0b00110000: dr = us; break;
		case 0b00110001: xr = us; break;
		case 0b00110010: yr = us; break;
		//case 0b00110011: us = us; break;
		case 0b00110100: sp = us; break;
		case 0b00110101: pc = us; break;

		case 0b01000000: dr = sp; break;
		case 0b01000001: xr = sp; break;
		case 0b01000010: yr = sp; break;
		case 0b01000011: us = sp; break;
		//case 0b01000100: sp = sp; break;
		case 0b01000101: pc = sp; break;

		case 0b01010000: dr = pc; break;
		case 0b01010001: xr = pc; break;
		case 0b01010010: yr = pc; break;
		case 0b01010011: us = pc; break;
		case 0b01010100: sp = pc; break;
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
	//
}

void mc6809::tsta(uint16_t ea)
{
	//
}

void mc6809::tstb(uint16_t ea)
{
	//
}
