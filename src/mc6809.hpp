/*
 * mc6809.hpp  -  part of MC6809
 *
 * (C)2021-2022 elmerucr
 */

/*
 * MC6809 version 0.9 - October 2021
 *
 * October 2021. Most functionality is in there. Currently missing:
 * - CWAI instruction
 * - SYNC instruction
 * - illegal opcode exception implementation
 *
 * Note:
 * Illegal opcode exception didn't exist in the real 6809. The idea is
 * taken from the Hitachi 6309. Will use the reserved vector at $fff0.
 */

#ifndef MC6809_HPP
#define MC6809_HPP

#include <cstdint>

#define MC6809_MAJOR_VERSION	0
#define MC6809_MINOR_VERSION	9
#define MC6809_BUILD		20211113
#define MC6809_YEAR		2021

#define	C_FLAG	0x01	// carry
#define	V_FLAG	0x02	// overflow
#define	Z_FLAG	0x04	// zero
#define	N_FLAG	0x08	// negative
#define	I_FLAG	0x10	// irq
#define	H_FLAG	0x20	// half carry
#define	F_FLAG	0x40	// firq
#define	E_FLAG	0x80	// entire state on stack

#define	VECTOR_ILL_OPC	0xfff0	// originally from 6309
#define	VECTOR_SWI3	0xfff2
#define	VECTOR_SWI2	0xfff4
#define	VECTOR_FIRQ	0xfff6
#define	VECTOR_IRQ	0xfff8
#define	VECTOR_SWI	0xfffa
#define	VECTOR_NMI	0xfffc
#define	VECTOR_RESET	0xfffe

class mc6809 {
public:
	/*
	 * read/write callbacks to memory bus
	 */
	typedef uint8_t (*bus_read)(uint16_t);
	typedef void (*bus_write)(uint16_t, uint8_t);
	bus_read read_8;
	bus_write write_8;

	/*
	 * Constructor receives function pointers for memory calls
	 */
	mc6809(bus_read r, bus_write w);

	~mc6809();

	/*
	 * Assignment of the different interrupt lines. The constructor of the
	 * cpu class creates true values (level up) by default - so if none
	 * is assigned, the cpu will still work.
	 */
	void assign_nmi_line(bool *line) { nmi_line = line; }
	void assign_firq_line(bool *line) { firq_line = line; }
	void assign_irq_line(bool *line) { irq_line = line; }

	/*
	 * Reset exception. Doesn't emulate the number of cycles taken.
	 */
	void reset();

	/*
	 * Main execute function. Runs one instruction, and returns the number
	 * of cycles consumed. Checking for breakpoints must be done with the
	 * breakpoint() member function that returns true or false.
	 */
	uint8_t execute();

	void status(char *text_buffer);
	void stacks(char *text_buffer, int no);
	uint16_t disassemble_instruction(char *buffer, uint16_t address);
	bool disassemble_successfull() { return disassemble_success; }

	inline bool is_e_flag_set()   { return (cc & E_FLAG) ? true  : false; }
	inline bool is_e_flag_clear() { return (cc & E_FLAG) ? false : true ; }
	inline bool is_f_flag_set()   { return (cc & F_FLAG) ? true  : false; }
	inline bool is_f_flag_clear() { return (cc & F_FLAG) ? false : true ; }
	inline bool is_h_flag_set()   { return (cc & H_FLAG) ? true  : false; }
	inline bool is_h_flag_clear() { return (cc & H_FLAG) ? false : true ; }
	inline bool is_i_flag_set()   { return (cc & I_FLAG) ? true  : false; }
	inline bool is_i_flag_clear() { return (cc & I_FLAG) ? false : true ; }
	inline bool is_n_flag_set()   { return (cc & N_FLAG) ? true  : false; }
	inline bool is_n_flag_clear() { return (cc & N_FLAG) ? false : true ; }
	inline bool is_z_flag_set()   { return (cc & Z_FLAG) ? true  : false; }
	inline bool is_z_flag_clear() { return (cc & Z_FLAG) ? false : true ; }
	inline bool is_v_flag_set()   { return (cc & V_FLAG) ? true  : false; }
	inline bool is_v_flag_clear() { return (cc & V_FLAG) ? false : true ; }
	inline bool is_c_flag_set()   { return (cc & C_FLAG) ? true  : false; }
	inline bool is_c_flag_clear() { return (cc & C_FLAG) ? false : true ; }

	inline void set_e_flag()   { cc |= E_FLAG; }
	inline void clear_e_flag() { cc &= (0xff - E_FLAG); }
	inline void set_f_flag()   { cc |= F_FLAG; }
	inline void clear_f_flag() { cc &= (0xff - F_FLAG); }
	inline void set_h_flag()   { cc |= H_FLAG; }
	inline void clear_h_flag() { cc &= (0xff - H_FLAG); }
	inline void set_i_flag()   { cc |= I_FLAG; }
	inline void clear_i_flag() { cc &= (0xff - I_FLAG); }
	inline void set_n_flag()   { cc |= N_FLAG; }
	inline void clear_n_flag() { cc &= (0xff - N_FLAG); }
	inline void set_z_flag()   { cc |= Z_FLAG; }
	inline void clear_z_flag() { cc &= (0xff - Z_FLAG); }
	inline void set_v_flag()   { cc |= V_FLAG; }
	inline void clear_v_flag() { cc &= (0xff - V_FLAG); }
	inline void set_c_flag()   { cc |= C_FLAG; }
	inline void clear_c_flag() { cc &= (0xff - C_FLAG); }

	inline void test_n_flag(uint8_t byte) { if (byte &  0x80) set_n_flag(); else clear_n_flag(); }
	inline void test_z_flag(uint8_t byte) { if (byte == 0x00) set_z_flag(); else clear_z_flag(); }
	inline void test_nz_flags(uint8_t byte) { test_n_flag(byte); test_z_flag(byte); }
	inline void test_n_flag_16(uint16_t word) { if (word &  0x8000) set_n_flag(); else clear_n_flag(); }
	inline void test_z_flag_16(uint16_t word) { if (word == 0x0000) set_z_flag(); else clear_z_flag(); }
	inline void test_nz_flags_16(uint16_t word) { test_n_flag_16(word); test_z_flag_16(word); }

	/*
	 * getters and setters, useful while debugging
	 * TODO: what about flags here?
	 */
	uint16_t get_pc()              { return pc; }
	void     set_pc(uint16_t word) { pc = word; }
	uint8_t  get_dp()              { return dp; }
	void     set_dp(uint8_t  byte) { dp = byte; }
	uint8_t  get_ac()              { return ac; }
	void     set_ac(uint8_t  byte) { ac = byte; }
	uint8_t  get_br()              { return br; }
	void     set_br(uint8_t  byte) { br = byte; }
	uint16_t get_dr()              { return (ac << 8) | br; }
	void     set_dr(uint16_t word) { ac = (word & 0xff00) >> 8; br = word & 0x00; }
	uint16_t get_xr()              { return xr; }
	void     set_xr(uint16_t word) { xr = word; }
	uint16_t get_yr()              { return yr; }
	void     set_yr(uint16_t word) { yr = word; }
	uint16_t get_us()              { return us; }
	void     set_us(uint16_t word) { us = word; }
	uint16_t get_sp()              { return sp; }
	void     set_sp(uint16_t word) { sp = word; }
	uint8_t  get_cc()              { return cc; }
	void     set_cc(uint8_t  byte) { cc = byte; }

	bool *breakpoint_array;
	inline bool breakpoint() { return breakpoint_array[pc] ? true : false; }
	void toggle_breakpoint(uint16_t address);
	void clear_breakpoints();

	inline uint32_t clock_ticks() { return cycles; }

private:
	uint16_t pc;	// program counter
	uint8_t	 dp;	// direct page register
	uint8_t  ac;	// accumulator a, msb of d
	uint8_t  br;	// accumulator b, lsb of d
	// uint16_t dr;	// non-existing = virtual as a combi of ac and br
	uint16_t xr;	// x index register
	uint16_t yr;	// y index register
	uint16_t us;	// user stack pointer
	uint16_t sp;	// hardware stack pointer
	uint8_t  cc;	// condition code register

	uint16_t *index_regs[4];

	bool nmi_enabled;
	bool default_pin;

	bool *nmi_line;
	bool old_nmi_line;
	bool *firq_line;
	bool old_firq_line;
	bool *irq_line;
	bool old_irq_line;

	int32_t cycle_saldo;
	uint32_t cycles;

	typedef uint16_t (mc6809::*addressing_mode)(bool *legal);
	typedef void (mc6809::*execute_instruction)(uint16_t);

	bool disassemble_success;

	/*
	 * Exception functions are private. Exceptions are triggered indirectly
	 * by the execute() function that polls the different interrupt lines
	 * or detects an illegal opcode.
	 */
	void nmi();
	void firq();
	void irq();
	void illegal_opcode();

	/*
	 * Internal stackpointer functionality
	 */
	inline void    push_sp(uint8_t byte) { (*write_8)(--sp, byte); }
	inline uint8_t pull_sp()             { return (*read_8)(sp++); }
	inline void    push_us(uint8_t byte) { (*write_8)(--us, byte); }
	inline uint8_t pull_us()             { return (*read_8)(us++); }

	/*
	 * addressing modes
	 */
	uint16_t a_dir(bool *legal);	// direct page (base page)
	uint16_t a_imb(bool *legal);	// immediate byte
	uint16_t a_imw(bool *legal);	// immediate word
	uint16_t a_ih(bool *legal);	// inherent
	uint16_t a_reb(bool *legal);	// relative byte (8 bit signed)
	uint16_t a_rew(bool *legal);	// relative word (16 bit signed)
	uint16_t a_idx(bool *legal);	// indexed
	uint16_t a_ext(bool *legal);	// extended
	uint16_t a_no(bool *legal);	// no mode (@ illegal instructions)

	// instructions
	void abx(uint16_t ea);
	void adca(uint16_t ea);
	void adcb(uint16_t ea);
	void adda(uint16_t ea);
	void addb(uint16_t ea);
	void addd(uint16_t ea);
	void anda(uint16_t ea);
	void andb(uint16_t ea);

	void andcc(uint16_t ea);
	void asl(uint16_t ea);		// also lsl
	void asla(uint16_t ea);
	void aslb(uint16_t ea);
	void asr(uint16_t ea);
	void asra(uint16_t ea);
	void asrb(uint16_t ea);
	void beq(uint16_t ea);

	void bge(uint16_t ea);
	void bgt(uint16_t ea);
	void bhi(uint16_t ea);
	void bhs(uint16_t ea);		// bcc
	void bita(uint16_t ea);
	void bitb(uint16_t ea);
	void bmi(uint16_t ea);
	void ble(uint16_t ea);

	void blo(uint16_t ea);		// bcs
	void bls(uint16_t ea);
	void blt(uint16_t ea);
	void bne(uint16_t ea);
	void bpl(uint16_t ea);
	void bra(uint16_t ea);
	void brn(uint16_t ea);
	void bsr(uint16_t ea);

	void bvc(uint16_t ea);
	void bvs(uint16_t ea);
	void clr(uint16_t ea);
	void clra(uint16_t ea);
	void clrb(uint16_t ea);
	void cmpa(uint16_t ea);
	void cmpb(uint16_t ea);
	void cmpd(uint16_t ea);

	void cmps(uint16_t ea);
	void cmpu(uint16_t ea);
	void cmpx(uint16_t ea);
	void cmpy(uint16_t ea);
	void com(uint16_t ea);
	void coma(uint16_t ea);
	void comb(uint16_t ea);
	void cwai(uint16_t ea);

	void daa(uint16_t ea);
	void dec(uint16_t ea);
	void deca(uint16_t ea);
	void decb(uint16_t ea);
	void eora(uint16_t ea);
	void eorb(uint16_t ea);
	void exg(uint16_t ea);
	void ill(uint16_t ea);		// illegal opcode (from 6309)

	void inc(uint16_t ea);
	void inca(uint16_t ea);
	void incb(uint16_t ea);
	void jmp(uint16_t ea);
	void jsr(uint16_t ea);
	void lbeq(uint16_t ea);
	void lbge(uint16_t ea);
	void lbgt(uint16_t ea);

	void lbhi(uint16_t ea);
	void lbhs(uint16_t ea);		// lbcc
	void lble(uint16_t ea);
	void lblo(uint16_t ea);		// lbcs
	void lbls(uint16_t ea);
	void lblt(uint16_t ea);
	void lbmi(uint16_t ea);
	void lbne(uint16_t ea);

	void lbpl(uint16_t ea);
	void lbra(uint16_t ea);
	void lbrn(uint16_t ea);
	void lbsr(uint16_t ea);
	void lbvc(uint16_t ea);
	void lbvs(uint16_t ea);
	void lda(uint16_t ea);
	void ldb(uint16_t ea);

	void ldd(uint16_t ea);
	void lds(uint16_t ea);
	void ldu(uint16_t ea);
	void ldx(uint16_t ea);
	void ldy(uint16_t ea);
	void leas(uint16_t ea);
	void leau(uint16_t ea);
	void leax(uint16_t ea);

	void leay(uint16_t ea);
	void lsr(uint16_t ea);
	void lsra(uint16_t ea);
	void lsrb(uint16_t ea);
	void mul(uint16_t ea);
	void neg(uint16_t ea);
	void nega(uint16_t ea);
	void negb(uint16_t ea);

	void nop(uint16_t ea);
	void ora(uint16_t ea);
	void orb(uint16_t ea);
	void orcc(uint16_t ea);

	void page2(uint16_t ea);
	void page3(uint16_t ea);

	void pshs(uint16_t ea);
	void pshu(uint16_t ea);
	void puls(uint16_t ea);
	void pulu(uint16_t ea);

	void rol(uint16_t ea);
	void rola(uint16_t ea);
	void rolb(uint16_t ea);
	void ror(uint16_t ea);
	void rora(uint16_t ea);
	void rorb(uint16_t ea);
	void rti(uint16_t ea);
	void rts(uint16_t ea);

	void sbca(uint16_t ea);
	void sbcb(uint16_t ea);
	void sex(uint16_t ea);
	void sta(uint16_t ea);
	void stb(uint16_t ea);
	void std(uint16_t ea);
	void sts(uint16_t ea);
	void stu(uint16_t ea);

	void stx(uint16_t ea);
	void sty(uint16_t ea);
	void suba(uint16_t ea);
	void subb(uint16_t ea);
	void subd(uint16_t ea);
	void swi(uint16_t ea);
	void swi2(uint16_t ea);
	void swi3(uint16_t ea);

	void sync(uint16_t ea);
	void tfr(uint16_t ea);
	void tst(uint16_t ea);
	void tsta(uint16_t ea);
	void tstb(uint16_t ea);

private:
	const execute_instruction opcodes_page1[256] = {
		&mc6809::neg,	&mc6809::ill,	&mc6809::ill,	&mc6809::com,	&mc6809::lsr,	&mc6809::ill,	&mc6809::ror,	&mc6809::asr,	// 0x00
		&mc6809::asl,	&mc6809::rol,	&mc6809::dec,	&mc6809::ill,	&mc6809::inc,	&mc6809::tst,	&mc6809::jmp,	&mc6809::clr,
		&mc6809::page2,	&mc6809::page3,	&mc6809::nop,	&mc6809::sync,	&mc6809::ill,	&mc6809::ill,	&mc6809::lbra,	&mc6809::lbsr,	// 0x10
		&mc6809::ill,	&mc6809::daa,	&mc6809::orcc,	&mc6809::ill,	&mc6809::andcc,	&mc6809::sex,	&mc6809::exg,	&mc6809::tfr,
		&mc6809::bra,	&mc6809::brn,	&mc6809::bhi,	&mc6809::bls,	&mc6809::bhs,	&mc6809::blo,	&mc6809::bne,	&mc6809::beq,	// 0x20
		&mc6809::bvc,	&mc6809::bvs,	&mc6809::bpl,	&mc6809::bmi,	&mc6809::bge,	&mc6809::blt,	&mc6809::bgt,	&mc6809::ble,
		&mc6809::leax,	&mc6809::leay,	&mc6809::leas,	&mc6809::leau,	&mc6809::pshs,	&mc6809::puls,	&mc6809::pshu,	&mc6809::pulu,	// 0x30
		&mc6809::ill,	&mc6809::rts,	&mc6809::abx,	&mc6809::rti,	&mc6809::cwai,	&mc6809::mul,	&mc6809::ill,	&mc6809::swi,
		&mc6809::nega,	&mc6809::ill,	&mc6809::ill,	&mc6809::coma,	&mc6809::lsra,	&mc6809::ill,	&mc6809::rora,	&mc6809::asra,	// 0x40
		&mc6809::asla,	&mc6809::rola,	&mc6809::deca,	&mc6809::ill,	&mc6809::inca,	&mc6809::tsta,	&mc6809::ill,	&mc6809::clra,
		&mc6809::negb,	&mc6809::ill,	&mc6809::ill,	&mc6809::comb,	&mc6809::lsrb,	&mc6809::ill,	&mc6809::rorb,	&mc6809::asrb,	// 0x50
		&mc6809::aslb,	&mc6809::rolb,	&mc6809::decb,	&mc6809::ill,	&mc6809::incb,	&mc6809::tstb,	&mc6809::ill,	&mc6809::clrb,
		&mc6809::neg,	&mc6809::ill,	&mc6809::ill,	&mc6809::com,	&mc6809::lsr,	&mc6809::ill,	&mc6809::ror,	&mc6809::asr,	// 0x60
		&mc6809::asl,	&mc6809::rol,	&mc6809::dec,	&mc6809::ill,	&mc6809::inc,	&mc6809::tst,	&mc6809::jmp,	&mc6809::clr,
		&mc6809::neg,	&mc6809::ill,	&mc6809::ill,	&mc6809::com,	&mc6809::lsr,	&mc6809::ill,	&mc6809::ror,	&mc6809::asr,	// 0x70
		&mc6809::asl,	&mc6809::rol,	&mc6809::dec,	&mc6809::ill,	&mc6809::inc,	&mc6809::tst,	&mc6809::jmp,	&mc6809::clr,
		&mc6809::suba,	&mc6809::cmpa,	&mc6809::sbca,	&mc6809::subd,	&mc6809::anda,	&mc6809::bita,	&mc6809::lda,	&mc6809::ill,	// 0x80
		&mc6809::eora,	&mc6809::adca,	&mc6809::ora,	&mc6809::adda,	&mc6809::cmpx,	&mc6809::bsr,	&mc6809::ldx,	&mc6809::ill,
		&mc6809::suba,	&mc6809::cmpa,	&mc6809::sbca,	&mc6809::subd,	&mc6809::anda,	&mc6809::bita,	&mc6809::lda,	&mc6809::sta,	// 0x90
		&mc6809::eora,	&mc6809::adca,	&mc6809::ora,	&mc6809::adda,	&mc6809::cmpx,	&mc6809::jsr,	&mc6809::ldx,	&mc6809::stx,
		&mc6809::suba,	&mc6809::cmpa,	&mc6809::sbca,	&mc6809::subd,	&mc6809::anda,	&mc6809::bita,	&mc6809::lda,	&mc6809::sta,	// 0xa0
		&mc6809::eora,	&mc6809::adca,	&mc6809::ora,	&mc6809::adda,	&mc6809::cmpx,	&mc6809::jsr,	&mc6809::ldx,	&mc6809::stx,
		&mc6809::suba,	&mc6809::cmpa,	&mc6809::sbca,	&mc6809::subd,	&mc6809::anda,	&mc6809::bita,	&mc6809::lda,	&mc6809::sta,	// 0xb0
		&mc6809::eora,	&mc6809::adca,	&mc6809::ora,	&mc6809::adda,	&mc6809::cmpx,	&mc6809::jsr,	&mc6809::ldx,	&mc6809::stx,
		&mc6809::subb,	&mc6809::cmpb,	&mc6809::sbcb,	&mc6809::addd,	&mc6809::andb,	&mc6809::bitb,	&mc6809::ldb,	&mc6809::ill,	// 0xc0
		&mc6809::eorb,	&mc6809::adcb,	&mc6809::orb,	&mc6809::addb,	&mc6809::ldd,	&mc6809::ill,	&mc6809::ldu,	&mc6809::ill,
		&mc6809::subb,	&mc6809::cmpb,	&mc6809::sbcb,	&mc6809::addd,	&mc6809::andb,	&mc6809::bitb,	&mc6809::ldb,	&mc6809::stb,	// 0xd0
		&mc6809::eorb,	&mc6809::adcb,	&mc6809::orb,	&mc6809::addb,	&mc6809::ldd,	&mc6809::std,	&mc6809::ldu,	&mc6809::stu,
		&mc6809::subb,	&mc6809::cmpb,	&mc6809::sbcb,	&mc6809::addd,	&mc6809::andb,	&mc6809::bitb,	&mc6809::ldb,	&mc6809::stb,	// 0xe0
		&mc6809::eorb,	&mc6809::adcb,	&mc6809::orb,	&mc6809::addb,	&mc6809::ldd,	&mc6809::std,	&mc6809::ldu,	&mc6809::stu,
		&mc6809::subb,	&mc6809::cmpb,	&mc6809::sbcb,	&mc6809::addd,	&mc6809::andb,	&mc6809::bitb,	&mc6809::ldb,	&mc6809::stb,	// 0xf0
		&mc6809::eorb,	&mc6809::adcb,	&mc6809::orb,	&mc6809::addb,	&mc6809::ldd,	&mc6809::std,	&mc6809::ldu,	&mc6809::stu
	};

	const execute_instruction opcodes_page2[256] = {
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x00
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x10
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::lbrn,	&mc6809::lbhi,	&mc6809::lbls,	&mc6809::lbhs,	&mc6809::lblo,	&mc6809::lbne,	&mc6809::lbeq,	// 0x20
		&mc6809::lbvc,	&mc6809::lbvs,	&mc6809::lbpl,	&mc6809::lbmi,	&mc6809::lbge,	&mc6809::lblt,	&mc6809::lbgt,	&mc6809::lble,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x30
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::swi2,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x40
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x50
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x60
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x70
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpd,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x80
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpy,	&mc6809::ill,	&mc6809::ldy,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpd,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x90
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpy,	&mc6809::ill,	&mc6809::ldy,	&mc6809::sty,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpd,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xa0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpy,	&mc6809::ill,	&mc6809::ldy,	&mc6809::sty,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpd,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xb0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpy,	&mc6809::ill,	&mc6809::ldy,	&mc6809::sty,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xc0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::lds,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xd0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::lds,	&mc6809::sts,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xe0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::lds,	&mc6809::sts,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xf0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::lds,	&mc6809::sts
	};

	const execute_instruction opcodes_page3[256] = {
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x00
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x10
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x20
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x30
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::swi3,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x40
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x50
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x60
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x70
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpu,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x80
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmps,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpu,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0x90
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmps,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpu,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xa0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmps,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmpu,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xb0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::cmps,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xc0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xd0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xe0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	// 0xf0
		&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill,	&mc6809::ill
	};

	const addressing_mode addressing_modes_page1[256] = {
		&mc6809::a_dir,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_no,	&mc6809::a_dir,	&mc6809::a_dir,	// 0x00
		&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_no,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,
		&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_rew,	&mc6809::a_rew,	// 0x10
		&mc6809::a_no,	&mc6809::a_ih,	&mc6809::a_imb,	&mc6809::a_no,	&mc6809::a_imb,	&mc6809::a_ih,	&mc6809::a_imb,	&mc6809::a_imb,
		&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	// 0x20
		&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,	&mc6809::a_reb,
		&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imb,	// 0x30
		&mc6809::a_no,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_no,	&mc6809::a_ih,
		&mc6809::a_ih,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_no,	&mc6809::a_ih,	&mc6809::a_ih,	// 0x40
		&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_no,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_no,	&mc6809::a_ih,
		&mc6809::a_ih,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_no,	&mc6809::a_ih,	&mc6809::a_ih,	// 0x50
		&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_no,	&mc6809::a_ih,	&mc6809::a_ih,	&mc6809::a_no,	&mc6809::a_ih,
		&mc6809::a_idx,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_no,	&mc6809::a_idx,	&mc6809::a_idx,	// 0x60
		&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_no,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,
		&mc6809::a_ext,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_no,	&mc6809::a_ext,	&mc6809::a_ext,	// 0x70
		&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_no,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,
		&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imw,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_no,	// 0x80
		&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imw,	&mc6809::a_reb,	&mc6809::a_imw,	&mc6809::a_no,
		&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	// 0x90
		&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,
		&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	// 0xa0
		&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,
		&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	// 0xb0
		&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,
		&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imw,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_no,	// 0xc0
		&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imb,	&mc6809::a_imw,	&mc6809::a_no,	&mc6809::a_imw,	&mc6809::a_no,
		&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	// 0xd0
		&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,	&mc6809::a_dir,
		&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	// 0xe0
		&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,	&mc6809::a_idx,
		&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	// 0xf0
		&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext,	&mc6809::a_ext
	};

	const addressing_mode addressing_modes_page2[256] = {
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x00
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x10
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,	// 0x20
		&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,	&mc6809::a_rew,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x30
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_ih,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x40
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x50
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x60
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x70
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_imw,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x80
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_imw,	&mc6809::a_no,	&mc6809::a_imw,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_dir,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x90
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_dir,	&mc6809::a_no,	&mc6809::a_dir,	&mc6809::a_dir,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_idx,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xa0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_idx,	&mc6809::a_no,	&mc6809::a_idx,	&mc6809::a_idx,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_ext,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xb0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_ext,	&mc6809::a_no,	&mc6809::a_ext,	&mc6809::a_ext,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xc0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_imw,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xd0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_dir,	&mc6809::a_dir,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xe0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_idx,	&mc6809::a_idx,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xf0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_ext,	&mc6809::a_ext
	};

	const addressing_mode addressing_modes_page3[256] = {
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x00
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x10
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x20
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x30
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_ih,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x40
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x50
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x60
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x70
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_imw,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x80
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_imw,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_dir,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0x90
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_dir,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_idx,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xa0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_idx,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_ext,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xb0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_ext,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xc0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xd0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xe0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	// 0xf0
		&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no,	&mc6809::a_no
	};

	const uint16_t cycles_page1[256] = {
		 6,  0,  0,  6,  6,  0,  6,  6,  6,  6,  6,  0,  6,  6,  3,  6,	// 0x00
		 0,  0,  2,  4,  0,  0,  5,  9,  0,  2,  3,  0,  3,  2,  8,  6,	// 0x10
		 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,	// 0x20
		 4,  4,  4,  4,  5,  5,  5,  5,  0,  5,  3,  6, 20, 11,  0, 19,	// 0x30
		 2,  0,  0,  2,  2,  0,  2,  2,  2,  2,  2,  0,  2,  2,  0,  2,	// 0x40
		 2,  0,  0,  2,  2,  0,  2,  2,  2,  2,  2,  0,  2,  2,  0,  2,	// 0x50
		 6,  0,  0,  6,  6,  0,  6,  6,  6,  6,  6,  0,  6,  6,  3,  6,	// 0x60
		 7,  0,  0,  7,  7,  0,  7,  7,  7,  7,  7,  0,  7,  7,  4,  7,	// 0x70
		 2,  2,  2,  4,  2,  2,  2,  0,  2,  2,  2,  2,  4,  7,  3,  0,	// 0x80
		 4,  4,  4,  6,  4,  4,  4,  4,  4,  4,  4,  4,  6,  7,  5,  5,	// 0x90
		 4,  4,  4,  6,  4,  4,  4,  4,  4,  4,  4,  4,  6,  7,  5,  5,	// 0xa0
		 5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  5,  7,  8,  6,  6,	// 0xb0
		 2,  2,  2,  4,  2,  2,  2,  0,  2,  2,  2,  2,  3,  0,  3,  0,	// 0xc0
		 4,  4,  4,  6,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,	// 0xd0
		 4,  4,  4,  6,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,	// 0xe0
		 5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6	// 0xf0
	};

	const uint16_t cycles_page2[256] = {
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x00
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x10
		 0,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,	// 0x20
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 20,	// 0x30
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x40
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x50
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x60
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x70
		 0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  5,  0,  4,  0,	// 0x80
		 0,  0,  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  7,  0,  6,  6,	// 0x90
		 0,  0,  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  7,  0,  6,  6,	// 0xa0
		 0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0,  7,  7,	// 0xb0
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,	// 0xc0
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  6,	// 0xd0
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  6,	// 0xe0
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7,  7	// 0xf0
	};

	const uint16_t cycles_page3[256] = {
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x00
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x10
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x20
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 20,	// 0x30
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x40
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x50
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x60
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0x70
		 0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  5,  0,  0,  0,	// 0x80
		 0,  0,  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  7,  0,  0,  0,	// 0x90
		 0,  0,  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  7,  0,  0,  0,	// 0xa0
		 0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0,  0,  0,	// 0xb0
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0xc0
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0xd0
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	// 0xe0
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0	// 0xf0
	};
};

#endif
