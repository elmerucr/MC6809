/*
 * mc6809dis.cpp  -  part of MC6809
 *
 * Code was borrowed from dasm09 which can be found at:
 * http://koti.mbnet.fi/~atjs/mc6809/Disassembler/dasm09.TGZ
 * Original copyright below
 * 
 * (c)2021 elmerucr
 */

/***************************************************************************
 * dasm09 -- Portable M6809/H6309/OS9 Disassembler                         *
 * Copyright (C) 2000  Arto Salmi                                          *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the Free Software             *
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               *
 ***************************************************************************/

#include "mc6809.hpp"
#include <cstdio>

enum mnemonics_index {
	_ABX=0,_ADCA, _ADCB, _ADDA, _ADDB, _ADDD, _ANDA, _ANDB,
	_ANDCC,_ASL,  _ASLA, _ASLB, _ASR,  _ASRA, _ASRB, _BEQ,
	_BGE,  _BGT,  _BHI,  _BHS,  _BITA, _BITB, _BMI,  _BLE,
	_BLO,  _BLS,  _BLT,  _BNE,  _BPL,  _BRA,  _BRN,  _BSR,
	_BVC,  _BVS,  _CLR,  _CLRA, _CLRB, _CMPA, _CMPB, _CMPD,
	_CMPS, _CMPU, _CMPX, _CMPY, _COM,  _COMA, _COMB, _CWAI,
	_DAA,  _DEC,  _DECA, _DECB, _EORA, _EORB, _EXG,  _ILL,
	_INC,  _INCA, _INCB, _JMP,  _JSR,  _LBEQ, _LBGE, _LBGT,
	_LBHI, _LBHS, _LBLE, _LBLO, _LBLS, _LBLT, _LBMI, _LBNE,
	_LBPL, _LBRA, _LBRN, _LBSR, _LBVC, _LBVS, _LDA,  _LDB,
	_LDD,  _LDS,  _LDU,  _LDX,  _LDY,  _LEAS, _LEAU, _LEAX,
	_LEAY, _LSR,  _LSRA, _LSRB, _MUL,  _NEG,  _NEGA, _NEGB,
	_NOP,  _ORA,  _ORB,  _ORCC, _PSHS, _PSHU, _PULS, _PULU,
	_ROL,  _ROLA, _ROLB, _ROR,  _RORA, _RORB, _RTI,  _RTS,
	_SBCA, _SBCB, _SEX,  _STA,  _STB,  _STD,  _STS,  _STU,
	_STX,  _STY,  _SUBA, _SUBB, _SUBD, _SWI,  _SWI2, _SWI3,
	_SYNC, _TFR,  _TST,  _TSTA, _TSTB
};

const char *mnemonics[133] = {
	"ABX",  "ADCA", "ADCB", "ADDA", "ADDB", "ADDD", "ANDA", "ANDB",
	"ANDCC","ASL",  "ASLA", "ASLB", "ASR",  "ASRA", "ASRB", "BEQ",
	"BGE",  "BGT",  "BHI",  "BHS",  "BITA", "BITB", "BMI",  "BLE",
	"BLO",  "BLS",  "BLT",  "BNE",  "BPL",  "BRA",  "BRN",  "BSR",
	"BVC",  "BVS",  "CLR",  "CLRA", "CLRB", "CMPA", "CMPB", "CMPD",
	"CMPS", "CMPU", "CMPX", "CMPY", "COM",  "COMA", "COMB", "CWAI",
	"DAA",  "DEC",  "DECA", "DECB", "EORA", "EORB", "EXG",  "ILL",
	"INC",  "INCA", "INCB", "JMP",  "JSR",  "LBEQ", "LBGE", "LBGT",
	"LBHI", "LBHS", "LBLE", "LBLO", "LBLS", "LBLT", "LBMI", "LBNE",
	"LBPL", "LBRA", "LBRN", "LBSR", "LBVC", "LBVS", "LDA",  "LDB",
	"LDD",  "LDS",  "LDU",  "LDX",  "LDY",  "LEAS", "LEAU", "LEAX",
	"LEAY", "LSR",  "LSRA", "LSRB", "MUL",  "NEG",  "NEGA", "NEGB",
	"NOP",  "ORA",  "ORB",  "ORCC", "PSHS", "PSHU", "PULS", "PULU",
	"ROL",  "ROLA", "ROLB", "ROR",  "RORA", "RORB", "RTI",  "RTS",
	"SBCA", "SBCB", "SEX",  "STA",  "STB",  "STD",  "STS",  "STU",
	"STX",  "STY",  "SUBA", "SUBB", "SUBD", "SWI",  "SWI2", "SWI3",
	"SYNC", "TFR",  "TST",  "TSTA", "TSTB"
};	    

uint16_t mc6809::disassemble_instruction(char *buffer, uint16_t address) {
	uint16_t start_address = address;
	uint8_t opcode = read_8(address++);

	if (opcode == 0x10) {
		// page 2
		opcode = read_8(address++);
	} else if (opcode == 0x11) {
		// page 3
		opcode = read_8(address++);
	} else {
		// page "1"

	}

	snprintf(buffer, 512, ",%04x %02x\n", start_address, opcode);
	return address - start_address;
}
