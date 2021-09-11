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

class memory;

class mc6809 {
private:
	uint8_t &A;	// accumulator A, msb of D
	uint8_t &B;	// accumulator B, lsb of D
	uint16_t D;	// 16bit accumulator D
	uint8_t	DP;	// direct page register
	uint8_t CC;	// condition code register

	uint16_t X;	// x index register
	uint16_t Y;	// y index register
	uint16_t U;	// user stack pointer
	uint16_t S;	// hardware stack pointer
	uint16_t PC;	// program counter

	bool nmi_allowed;

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
};

#endif
