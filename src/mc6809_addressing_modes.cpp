/*
 * mc6809_addressing_modes.cpp  -  part of MC6809
 *
 * (C)2021-2022 elmerucr
 */

#include "mc6809.hpp"

uint16_t mc6809::a_dir(bool *legal)
{
	*legal = true;
	return (dp << 8) | (*read_8)(pc++);
}

uint16_t mc6809::a_ih(bool *legal)
{
	// Inherent, instruction contains all information.
	*legal = true;
	return 0;
}

uint16_t mc6809::a_imb(bool *legal)
{
	*legal = true;
	return pc++;
}

uint16_t mc6809::a_imw(bool *legal)
{
	uint16_t address = pc++;
	pc++;
	*legal = true;
	return address;
}

uint16_t mc6809::a_reb(bool *legal)
{
	// sign extend the 8 bit value
	uint16_t offset = (uint16_t)((int8_t)(*read_8)(pc++));
	*legal = true;
	return (uint16_t)(pc + offset);
}

uint16_t mc6809::a_rew(bool *legal)
{
	uint16_t offset = (*read_8)(pc++);
	offset = (offset << 8) | (*read_8)(pc++);
	*legal = true;
	return pc + offset;
}

uint16_t mc6809::a_idx(bool *legal)
{
	/*
	 * First, assume the addressing mode is legal. If not, this
	 * flag will be changed in the code below.
	 */
	*legal = true;

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

					offset = (ac << 8) | br;
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
				case 0b1101:
					/*
					 * constant offset pc 16bit, reads 2 extra bytes
					 */
					cycles += 5;

					offset = (*read_8)(pc++) << 8;
					offset |= (*read_8)(pc++);
					address = pc + offset;
					break;
				default:
					// TODO
					// all others are illegal
					*legal = false;
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
				case 0b1001:
					/*
					 * indirect, 16-bit offset
					 */
					cycles += 7;

					offset = (*read_8)(pc++) << 8;
					offset |= (*read_8)(pc++);
					word = *index_regs[(postbyte & 0b01100000) >> 5]
						+ offset;
					address = (*read_8)(word++) << 8;
					address |= (*read_8)(word);
					break;
				case 0b0110:
					/*
					 * indirect accumulator a offset
					 */
					cycles += 4;

					if (ac & 0b10000000) {
						offset = 0xff00 | ac;
					} else {
						offset = ac;
					}
					word = *index_regs[(postbyte & 0b01100000) >> 5]
						+ offset;
					address = (*read_8)(word++) << 8;
					address |= (*read_8)(word);
					break;
				case 0b0101:
					/*
					 * indirect accumulator b offset
					 */
					cycles += 4;

					if (br & 0b10000000) {
						offset = 0xff00 | br;
					} else {
						offset = br;
					}
					word = *index_regs[(postbyte & 0b01100000) >> 5]
						+ offset;
					address = (*read_8)(word++) << 8;
					address |= (*read_8)(word);
					break;
				case 0b1011:
					/*
					 * accumulator d offset
					 */
					cycles += 7;

					offset = (ac << 8) | br;
					word = *index_regs[(postbyte & 0b01100000) >> 5]
						+ offset;
					address = (*read_8)(word++) << 8;
					address |= (*read_8)(word);
					break;
				case 0b0001:
					/*
					 * indirect auto increment by 2
					 */
					cycles += 6;

					word = *index_regs[(postbyte & 0b01100000) >> 5];
					(*index_regs[(postbyte & 0b01100000) >> 5]) += 2;
					address = (*read_8)(word++) << 8;
					address |= (*read_8)(word);
					break;
				case 0b0011:
					/*
					 * indirect auto decrement by 2
					 */
					cycles += 6;

					(*index_regs[(postbyte & 0b01100000) >> 5]) -= 2;
					word = *index_regs[(postbyte & 0b01100000) >> 5];
					address = (*read_8)(word++) << 8;
					address |= (*read_8)(word);
					break;
				case 0b1100:
					/*
					 * indirect constant offset pc 8bit, reads extra byte
					 */
					cycles += 4;

					byte = (*read_8)(pc++);
					if (byte & 0b10000000) {
						offset = 0xff00 | byte;
					} else {
						offset = byte;
					}
					word = pc + offset;
					address = (*read_8)(word++) << 8;
					address |= (*read_8)(word);
					break;
				case 0b1101:
					/*
					 * indirect constant offset pc 16bit, reads 2 extra bytes
					 */
					cycles += 8;

					offset = (*read_8)(pc++) << 8;
					offset |= (*read_8)(pc++);
					word = pc + offset;
					address = (*read_8)(word++) << 8;
					address |= (*read_8)(word);
					break;
				default:
					// TODO
					// all others are illegal
					*legal = false;
					break;
				}
				break;
			}
			break;
		};
	}
	return address;
}

uint16_t mc6809::a_ext(bool *legal)
{
	uint16_t word = ((*read_8)(pc++)) << 8;
	word |= (*read_8)(pc++);
	*legal = true;
	return word;
}

uint16_t mc6809::a_no(bool *legal)
{
	// no mode @ illegal instruction
	*legal = false;
	return 0;
}
