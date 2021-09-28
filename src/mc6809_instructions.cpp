/*
 * mc6809_opcodes.cpp  -  part of MC6809
 *
 * (c)2021 elmerucr
 */

#include "mc6809.hpp"
#include <cstdio>

void mc6809::ill(uint16_t ea)
{
	// "NEW": from 6309
	// push all registers, load vector illegal opcode ....
}

void mc6809::abx(uint16_t ea)
{
	//
}

void mc6809::adca(uint16_t ea)
{
	//
}

void mc6809::adcb(uint16_t ea)
{
	//
}

void mc6809::adda(uint16_t ea)
{
	//
}

void mc6809::addb(uint16_t ea)
{
	//
}

void mc6809::addd(uint16_t ea)
{
	//
}

void mc6809::anda(uint16_t ea)
{
	//
}

void mc6809::andb(uint16_t ea)
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
	pc = ea;
}

void mc6809::brn(uint16_t ea)
{
	//
}

void mc6809::bsr(uint16_t ea)
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

void mc6809::clra(uint16_t ea)
{
	//
}

void mc6809::clrb(uint16_t ea)
{
	//
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
	//
}

void mc6809::eorb(uint16_t ea)
{
	//
}

void mc6809::exg(uint16_t ea)
{
	//
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
	//
}

void mc6809::lbeq(uint16_t ea)
{
	//
}

void mc6809::lbge(uint16_t ea)
{
	//
}

void mc6809::lbgt(uint16_t ea)
{
	//
}

void mc6809::lbhi(uint16_t ea)
{
	//
}

void mc6809::lbhs(uint16_t ea)
{
	//
}

void mc6809::lble(uint16_t ea)
{
	//
}

void mc6809::lblo(uint16_t ea)
{
	//
}

void mc6809::lbls(uint16_t ea)
{
	//
}

void mc6809::lblt(uint16_t ea)
{
	//
}

void mc6809::lbmi(uint16_t ea)
{
	//
}

void mc6809::lbne(uint16_t ea)
{
	//
}

void mc6809::lbpl(uint16_t ea)
{
	//
}

void mc6809::lbra(uint16_t ea)
{
	pc = ea;
}

void mc6809::lbrn(uint16_t ea)
{
	//
}

void mc6809::lbsr(uint16_t ea)
{
	//
}

void mc6809::lbvc(uint16_t ea)
{
	//
}

void mc6809::lbvs(uint16_t ea)
{
	//
}

void mc6809::lda(uint16_t ea)
{
	//
}

void mc6809::ldb(uint16_t ea)
{
	//
}

void mc6809::ldd(uint16_t ea)
{
	//
}

void mc6809::lds(uint16_t ea)
{
	//
}

void mc6809::ldu(uint16_t ea)
{
	//
}

void mc6809::ldx(uint16_t ea)
{
	//
}

void mc6809::ldy(uint16_t ea)
{
	//
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
	//
}

void mc6809::ora(uint16_t ea)
{
	//
}

void mc6809::orb(uint16_t ea)
{
	//
}

void mc6809::orcc(uint16_t ea)
{
	//
}

void mc6809::page2(uint16_t ea)
{
	uint8_t opcode = (*read_8)(pc++);
	cycles += cycles_page2[opcode];

	uint16_t effective_address = (this->*addressing_modes_page2[opcode])();
	(this->*opcodes_page2[opcode])(effective_address);
}

void mc6809::page3(uint16_t ea)
{
	uint8_t opcode = (*read_8)(pc++);
	cycles += cycles_page3[opcode];

	uint16_t effective_address = (this->*addressing_modes_page3[opcode])();
	(this->*opcodes_page3[opcode])(effective_address);
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
	//
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
	//
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
