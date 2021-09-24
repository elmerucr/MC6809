/*
 * mc6809dis.cpp  -  part of MC6809
 *
 * (c)2021 elmerucr
 */

#include "mc6809.hpp"
#include <cstdio>

int mc6809::disassemble_instruction(char *buffer, uint16_t address) {
	snprintf(buffer, 512, ",%04x %02x\n", address, read_8(address));
	return 0;
}
