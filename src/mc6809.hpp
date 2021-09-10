#ifndef MC6809_HPP
#define MC6809_HPP

#include <cstdint>

class mc6809 {
private:
	uint16_t D;
	uint8_t B;
	uint8_t (*read)(uint16_t address);
	void (*write)(uint16_t address, uint8_t byte);
public:
	typedef void (mc6809::*execute_opcode)(void);
	mc6809();
	bool run(uint16_t cycles);
	void lda();
	void sta();
	execute_opcode opcodes[2];
};

#endif