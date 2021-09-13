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

#define	VECTOR_EMPTY	0xfff0
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
	uint16_t pcr;	// program counter
	uint8_t &a;	// accumulator a, msb of d
	uint8_t &b;	// accumulator b, lsb of d
	uint16_t dr;	// 16bit accumulator d
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
	void lda();
	void sta();
	typedef void (mc6809::*execute_opcode)(void);
	execute_opcode opcodes[2];
	void status();
};

#endif
