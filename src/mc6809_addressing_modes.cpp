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

uint16_t mc6809::a_imb()
{
	return pc++;
}

uint16_t mc6809::a_imw()
{
	uint16_t address = pc++;
	pc++;
	return address;
}

uint16_t mc6809::a_reb()
{
	// sign extend the 8 bit value
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
	uint16_t offset;
	uint8_t byte;
	uint16_t word;

	// read postbyte
	uint8_t postbyte = (*read_8)(pc++);

	if (postbyte == 0b10011111) {
		/*
		 * indirect extended
		 */
		cycles += 5;

		word = (*read_8)(pc++) << 8;
		word |= (*read_8)(pc++);
		address = (*read_8)(word++) << 8;
		address |= (*read_8)(word);
	} else {
		switch (postbyte & 0b10000000) {
		case 0b00000000:
			/*
			 * non-indirect, constant 5-bit signed offset
			 */
			cycles += 1;

			uint16_t offset;
			if (postbyte & 0b00010000) {
				offset = 0xffe0 | (postbyte & 0x1f);
			} else {
				offset = postbyte & 0x0f; // bit 4 always 0
			}
			address = *index_regs[(postbyte & 0b01100000) >> 5]
				+ offset;
			break;
		case 0b10000000:
			/*
			 * all other modes
			 */

			switch (postbyte & 0b00010000) {
			case 0b00000000:
				/*
				 * non-indirect
				 */

				switch (postbyte & 0b00001111) {
				case 0b0100:
					/*
					 * no offset
					 * uses no extra cycles
					 */

					address = *index_regs[(postbyte & 0b01100000) >> 5];
					break;
				case 0b1000:
					/*
					 * 8-bit offset
					 */
					cycles += 1;

					byte = (*read_8)(pc++);
					if (byte & 0b10000000) {
						offset = 0xff00 | byte;
					} else {
						offset = byte;
					}
					address = *index_regs[(postbyte & 0b01100000) >> 5]
						+ offset;
					break;
				case 0b1001:
					/*
					 * 16 bit offset
					 */
					cycles += 4;

					offset = (*read_8)(pc++) << 8;
					offset |= (*read_8)(pc++);
					address = *index_regs[(postbyte & 0b01100000) >> 5]
						+ offset;
					break;
				case 0b0110:
					/*
					 * accumulator a offset
					 */
					cycles += 1;

					if (ac & 0b10000000) {
						offset = 0xff00 | ac;
					} else {
						offset = ac;
					}
					address = *index_regs[(postbyte & 0b01100000) >> 5]
						+ offset;
					break;
				case 0b0101:
					/*
					 * accumulator b offset
					 */
					cycles += 1;

					if (br & 0b10000000) {
						offset = 0xff00 | br;
					} else {
						offset = br;
					}
					address = *index_regs[(postbyte & 0b01100000) >> 5]
						+ offset;
					break;
				case 0b1011:
					/*
					 * accumulator d offset
					 */
					cycles += 4;

					offset = dr;
					address = *index_regs[(postbyte & 0b01100000) >> 5]
						+ offset;
					break;
				case 0b0000:
					/*
					 * auto increment by 1
					 */
					cycles += 2;

					address = *index_regs[(postbyte & 0b01100000) >> 5];
					(*index_regs[(postbyte & 0b01100000) >> 5])++;
					break;
				case 0b0001:
					/*
					 * auto increment by 2
					 */
					cycles += 3;

					address = *index_regs[(postbyte & 0b01100000) >> 5];
					(*index_regs[(postbyte & 0b01100000) >> 5]) += 2;
					break;
				case 0b0010:
					/*
					 * auto decrement by 1
					 */
					cycles += 2;

					(*index_regs[(postbyte & 0b01100000) >> 5])--;
					address = *index_regs[(postbyte & 0b01100000) >> 5];
					break;
				case 0b0011:
					/*
					 * auto decrement by 2
					 */
					cycles += 3;

					(*index_regs[(postbyte & 0b01100000) >> 5]) -= 2;
					address = *index_regs[(postbyte & 0b01100000) >> 5];
					break;
				case 0b1100:
					/*
					 * constant offset pc 8bit, reads extra byte
					 */
					cycles += 1;

					byte = (*read_8)(pc++);
					if (byte & 0b10000000) {
						offset = 0xff00 | byte;
					} else {
						offset = byte;
					}
					address = pc + offset;
					break;
				default:
					// TODO
					// all others are illegal
					break;
				}
				break;
			case 0b00010000:
				/*
				 * indirect
				 */

				switch (postbyte & 0b00001111) {
				case 0b0100:
					/*
					 * indirect, no offset
					 */
					cycles += 3;

					word = *index_regs[(postbyte & 0b01100000) >> 5];
					address = (*read_8)(word++) << 8;
					address |= (*read_8)(word);
					break;
				case 0b1000:
					/*
					 * indirect, 8-bit offset
					 */
					cycles += 4;

					byte = (*read_8)(pc++);
					if (byte & 0b10000000) {
						offset = 0xff00 | byte;
					} else {
						offset = byte;
					}
					word = *index_regs[(postbyte & 0b01100000) >> 5]
						+ offset;
					address = (*read_8)(word++) << 8;
					address |= (*read_8)(word);
					break;
				default:
					// TODO
					// all others are illegal
					break;
				}
				break;
			}
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
