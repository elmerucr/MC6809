/*
 * mc6809_addressing_modes.cpp  -  part of MC6809
 *
 * (c)2021 elmerucr
 */

#include "mc6809.hpp"

uint16_t mc6809::a_dir()
{
	return (dp << 8) | (*read_8)(pc++);
}

uint16_t mc6809::a_ih()
{
	// Inherent, instruction contains all information.
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
	uint16_t address = 0;

	// read postbyte
	uint8_t postbyte = (*read_8)(pc++);

	if (postbyte == 0b10011111) {
		// indirect extended
		cycles += 5;
		uint16_t word = (*read_8)(pc++) << 8;
		word |= (*read_8)(pc++);
		address = (*read_8)(word++) << 8;
		address |= (*read_8)(word);
	} else {
		switch (postbyte & 0b00000000) {
			case 0:
				break;
			case 1:
				break;
		};
	}
	return address;
}

uint16_t mc6809::a_ext()
{
	uint16_t word = ((*read_8)(pc++)) << 8;
	word |= (*read_8)(pc++);
	return word;
}

uint16_t mc6809::a_no()
{
	// no mode @ illegal instruction
	return 0;
}
