#ifndef MC6809_HPP
#define MC6809_HPP

#include <cstdint>

#define	C_FLAG	0x01	// carry
#define	V_FLAG	0x02	// overflow
#define	Z_FLAG	0x04	// zero
#define	N_FLAG	0x08	// negative
#define	I_FLAG	0x10	// irq
#define	H_FLAG	0x20	// half carry
#define	F_FLAG	0x40	// firq
#define	E_FLAG	0x80	// entire state on stack

#define	VECTOR_ILL_OPC	0xfff0	// this one is added for convenience
#define	VECTOR_SWI3	0xfff2
#define	VECTOR_SWI2	0xfff4
#define	VECTOR_FIRQ	0xfff6
#define	VECTOR_IRQ	0xfff8
#define	VECTOR_SWI	0xfffa
#define	VECTOR_NMI	0xfffc
#define	VECTOR_RESET	0xfffe

class memory;

class mc6809 {
private:
	uint16_t xr;	// x index register
	uint16_t yr;	// y index register
	uint16_t us;	// user stack pointer
	uint16_t sp;	// hardware stack pointer
	uint16_t pc;	// program counter
	uint8_t &a;	// accumulator a, msb of d
	uint8_t &b;	// accumulator b, lsb of d
	uint16_t d;	// 16bit accumulator d
	uint8_t	 dpr;	// direct page register
	uint8_t  ccr;	// condition code register

	bool nmi_blocked;

	bool *nmi_line;
	bool old_nmi_line;
	bool *firq_line;
	bool old_firq_line;
	bool *irq_line;
	bool old_irq_line;

	// read write callbacks
	typedef uint8_t (*bus_read)(uint16_t);
	typedef void (*bus_write)(uint16_t, uint8_t);
	bus_read read_8;
	bus_write write_8;
public:
	mc6809(bus_read r, bus_write w);
	void reset();
	bool run(uint16_t cycles);

	// addressing modes
	uint16_t inh();
	uint16_t imm();

	// instructions
	void abx(uint16_t ea);
	void andcc(uint16_t ea);
	void asl(uint16_t ea);
	void asr(uint16_t ea);
	void beq(uint16_t ea);
	void bge(uint16_t ea);
	void bgt(uint16_t ea);
	void bhi(uint16_t ea);
	void bhs(uint16_t ea);
	void bmi(uint16_t ea);
	void ble(uint16_t ea);
	void blo(uint16_t ea);
	void bls(uint16_t ea);
	void blt(uint16_t ea);
	void bne(uint16_t ea);
	void bpl(uint16_t ea);
	void bra(uint16_t ea);
	void brn(uint16_t ea);
	void bvc(uint16_t ea);
	void bvs(uint16_t ea);
	void clr(uint16_t ea);
	void com(uint16_t ea);
	void cwai(uint16_t ea);
	void daa(uint16_t ea);
	void dec(uint16_t ea);
	void exg(uint16_t ea);
	void ill(uint16_t ea);		// illegal opcode
	void inc(uint16_t ea);
	void jmp(uint16_t ea);
	void lbra(uint16_t ea);
	void lbsr(uint16_t ea);
	void lda(uint16_t ea);
	void leax(uint16_t);
	void leay(uint16_t);
	void leas(uint16_t);
	void leau(uint16_t);
	void lsr(uint16_t ea);
	void mul(uint16_t ea);
	void neg(uint16_t ea);
	void nop(uint16_t ea);
	void orcc(uint16_t ea);
	void page2(uint16_t ea);
	void page3(uint16_t ea);
	void pshs(uint16_t ea);
	void pshu(uint16_t ea);
	void puls(uint16_t ea);
	void pulu(uint16_t ea);
	void rol(uint16_t ea);
	void ror(uint16_t ea);
	void rts(uint16_t ea);
	void sex(uint16_t ea);
	void sta(uint16_t ea);
	void swi(uint16_t ea);
	void sync(uint16_t ea);
	void tfr(uint16_t ea);
	void tst(uint16_t ea);

	typedef uint16_t (mc6809::*addressing_mode)();
	typedef void (mc6809::*execute_instruction)(uint16_t);
	void status();
private:
	addressing_mode addressing_modes[256] = {
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,
		&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh,	&mc6809::inh
	};

	execute_instruction opcodes[256] = {
		&mc6809::neg,	&mc6809::ill,	&mc6809::ill,	&mc6809::com,	&mc6809::lsr,	&mc6809::ill,	&mc6809::ror,	&mc6809::asr,	// 0x00
		&mc6809::asl,	&mc6809::rol,	&mc6809::dec,	&mc6809::ill,	&mc6809::inc,	&mc6809::tst,	&mc6809::jmp,	&mc6809::clr,
		&mc6809::page2,	&mc6809::page3,	&mc6809::nop,	&mc6809::sync,	&mc6809::ill,	&mc6809::ill,	&mc6809::lbra,	&mc6809::lbsr,	// 0x10
		&mc6809::ill,	&mc6809::daa,	&mc6809::orcc,	&mc6809::ill,	&mc6809::andcc,	&mc6809::sex,	&mc6809::exg,	&mc6809::tfr,
		&mc6809::bra,	&mc6809::brn,	&mc6809::bhi,	&mc6809::bls,	&mc6809::bhs,	&mc6809::blo,	&mc6809::bne,	&mc6809::beq,	// 0x20
		&mc6809::bvc,	&mc6809::bvs,	&mc6809::bpl,	&mc6809::bmi,	&mc6809::bge,	&mc6809::blt,	&mc6809::bgt,	&mc6809::ble,
		&mc6809::leax,	&mc6809::leay,	&mc6809::leas,	&mc6809::leau,	&mc6809::pshs,	&mc6809::puls,	&mc6809::pshu,	&mc6809::pulu,	// 0x30
		&mc6809::ill,	&mc6809::rts,	&mc6809::abx,	&mc6809::rts,	&mc6809::cwai,	&mc6809::mul,	&mc6809::ill,	&mc6809::swi,
		&mc6809::neg,	&mc6809::ill,	&mc6809::ill,	&mc6809::com,	&mc6809::lsr,	&mc6809::ill,	&mc6809::ror,	&mc6809::asr,	// 0x40
		&mc6809::asl,	&mc6809::rol,	&mc6809::dec,	&mc6809::ill,	&mc6809::inc,	&mc6809::tst,	&mc6809::ill,	&mc6809::clr,
		&mc6809::neg,	&mc6809::ill,	&mc6809::ill,	&mc6809::com,	&mc6809::lsr,	&mc6809::ill,	&mc6809::ror,	&mc6809::asr,	// 0x50
		&mc6809::asl,	&mc6809::rol,	&mc6809::dec,	&mc6809::ill,	&mc6809::inc,	&mc6809::tst,	&mc6809::ill,	&mc6809::clr,
		&mc6809::neg,	&mc6809::ill,	&mc6809::ill,	&mc6809::com,	&mc6809::lsr,	&mc6809::ill,	&mc6809::ror,	&mc6809::asr,	// 0x60
		&mc6809::asl,	&mc6809::rol,	&mc6809::dec,	&mc6809::ill,	&mc6809::inc,	&mc6809::tst,	&mc6809::jmp,	&mc6809::clr,
		&mc6809::neg,	&mc6809::ill,	&mc6809::ill,	&mc6809::com,	&mc6809::lsr,	&mc6809::ill,	&mc6809::ror,	&mc6809::asr,	// 0x70
		&mc6809::asl,	&mc6809::rol,	&mc6809::dec,	&mc6809::ill,	&mc6809::inc,	&mc6809::tst,	&mc6809::jmp,	&mc6809::clr,

		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x80
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x90
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xa0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xb0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xc0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xd0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xe0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xf0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill
	};
};

#endif
