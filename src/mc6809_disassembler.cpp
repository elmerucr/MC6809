/*
 * mc6809_disassembler.cpp  -  part of MC6809
 *
 * (c)2021 elmerucr
 *
 * Code is inspired by dasm09 which can be found at:
 * http://koti.mbnet.fi/~atjs/mc6809/Disassembler/dasm09.TGZ
 * And by f9dasm:
 * https://github.com/Arakula/f9dasm
 */

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

enum addr_mode_index {
	_NOM,	// no mode
	_IMP,	// inherent/implied
	_IMB,	// immediate byte
	_IMW,	// immediate word
	_DIR,	// direct
	_EXT,	// extended
	_IND,	// indexed
	_REB,	// relative byte
	_REW,	// relative word
	_R1,	// tfr/exg mode
	_R2,	// pul/psh system
	_R3,	// pul/psh user
	_BD,	// Bit Manipulation direct
	_BI,	// Bit Manipulation index
	_BE,	// Bit Manipulation extended
	_BT,	// Bit Transfers direct
	_T1,	// Block Transfer r0+,r1+
	_T2,	// Block Transfer r0-,r1-
	_T3,	// Block Transfer r0+,r1
	_T4,	// Block Transfer r0,r1+
	_IML 	// immediate 32-bit
};

const char *mnemonics[133] = {
	"abx  ","adca ","adcb ","adda ","addb ","addd ","anda ","andb ",
	"andcc","asl  ","asla ","aslb ","asr  ","asra ","asrb ","beq  ",
	"bge  ","bgt  ","bhi  ","bhs  ","bita ","bitb ","bmi  ","ble  ",
	"blo  ","bls  ","blt  ","bne  ","bpl  ","bra  ","brn  ","bsr  ",
	"bvc  ","bvs  ","clr  ","clra ","clrb ","cmpa ","cmpb ","cmpd ",
	"cmps ","cmpu ","cmpx ","cmpy ","com  ","coma ","comb ","cwai ",
	"daa  ","dec  ","deca ","decb ","eora ","eorb ","exg  ","ill  ",
	"inc  ","inca ","incb ","jmp  ","jsr  ","lbeq ","lbge ","lbgt ",
	"lbhi ","lbhs ","lble ","lblo ","lbls ","lblt ","lbmi ","lbne ",
	"lbpl ","lbra ","lbrn ","lbsr ","lbvc ","lbvs ","lda  ","ldb  ",
	"ldd  ","lds  ","ldu  ","ldx  ","ldy  ","leas ","leau ","leax ",
	"leay ","lsr  ","lsra ","lsrb ","mul  ","neg  ","nega ","negb ",
	"nop  ","ora  ","orb  ","orcc ","pshs ","pshu ","puls ","pulu ",
	"rol  ","rola ","rolb ","ror  ","rora ","rorb ","rti  ","rts  ",
	"sbca ","sbcb ","sex  ","sta  ","stb  ","std  ","sts  ","stu  ",
	"stx  ","sty  ","suba ","subb ","subd ","swi  ","swi2 ","swi3 ",
	"sync ","tfr  ","tst  ","tsta ","tstb "
};

enum mnemonics_index opcodes_page_1[256] = {
	_NEG,	_ILL,	_ILL,	_COM,	_LSR,	_ILL,	_ROR,	_ASR,	// 0x00
	_ASL,	_ROL,	_DEC,	_ILL,	_INC,	_TST,	_JMP,	_CLR,
	_ILL,	_ILL,	_NOP,	_SYNC,	_ILL,	_ILL,	_LBRA,	_LBSR,	// 0x10
	_ILL,	_DAA,	_ORCC,	_ILL,	_ANDCC,	_SEX,	_EXG,	_TFR,
	_BRA,	_BRN,	_BHI,	_BLS,	_BHS,	_BLO,	_BNE,	_BEQ,	// 0x20
	_BVC,	_BVS,	_BPL,	_BMI,	_BGE,	_BLT,	_BGT,	_BLE,
	_LEAX,	_LEAY,	_LEAS,	_LEAU,	_PSHS,	_PULS,	_PSHU,	_PULU,	// 0x30
	_ILL,	_RTS,	_ABX,	_RTI,	_CWAI,	_MUL,	_ILL,	_SWI,
	_NEGA,	_ILL,	_ILL,	_COMA,	_LSRA,	_ILL,	_RORA,	_ASRA,	// 0x40
	_ASLA,	_ROLA,	_DECA,	_ILL,	_INCA,	_TSTA,	_ILL,	_CLRA,
	_NEGB,	_ILL,	_ILL,	_COMB,	_LSRB,	_ILL,	_RORB,	_ASRB,	// 0x50
	_ASLB,	_ROLB,	_DECB,	_ILL,	_INCB,	_TSTB,	_ILL,	_CLRB,
	_NEG,	_ILL,	_ILL,	_COM,	_LSR,	_ILL,	_ROR,	_ASR,	// 0x60
	_ASL,	_ROL,	_DEC,	_ILL,	_INC,	_TST,	_JMP,	_CLR,
	_NEG,	_ILL,	_ILL,	_COM,	_LSR,	_ILL,	_ROR,	_ASR,	// 0x70
	_ASL,	_ROL,	_DEC,	_ILL,	_INC,	_TST,	_JMP,	_CLR,
	_SUBA,	_CMPA,	_SBCA,	_SUBD,	_ANDA,	_BITA,	_LDA,	_ILL,	// 0x80
	_EORA,	_ADCA,	_ORA,	_ADDA,	_CMPX,	_BSR,	_LDX,	_ILL,
	_SUBA,	_CMPA,	_SBCA,	_SUBD,	_ANDA,	_BITA,	_LDA,	_STA,	// 0x90
	_EORA,	_ADCA,	_ORA,	_ADDA,	_CMPX,	_JSR,	_LDX,	_STX,
	_SUBA,	_CMPA,	_SBCA,	_SUBD,	_ANDA,	_BITA,	_LDA,	_STA,	// 0xa0
	_EORA,	_ADCA,	_ORA,	_ADDA,	_CMPX,	_JSR,	_LDX,	_STX,
	_SUBA,	_CMPA,	_SBCA,	_SUBD,	_ANDA,	_BITA,	_LDA,	_STA,	// 0xb0
	_EORA,	_ADCA,	_ORA,	_ADDA,	_CMPX,	_JSR,	_LDX,	_STX,
	_SUBB,	_CMPB,	_SBCB,	_ADDD,	_ANDB,	_BITB,	_LDB,	_ILL,	// 0xc0
	_EORB,	_ADCB,	_ORB,	_ADDB,	_LDD,	_ILL,	_LDU,	_ILL,
	_SUBB,	_CMPB,	_SBCB,	_ADDD,	_ANDB,	_BITB,	_LDB,	_STB,	// 0xd0
	_EORB,	_ADCB,	_ORB,	_ADDB,	_LDD,	_STD,	_LDU,	_STU,
	_SUBB,	_CMPB,	_SBCB,	_ADDD,	_ANDB,	_BITB,	_LDB,	_STB,	// 0xe0
	_EORB,	_ADCB,	_ORB,	_ADDB,	_LDD,	_STD,	_LDU,	_STU,
	_SUBB,	_CMPB,	_SBCB,	_ADDD,	_ANDB,	_BITB,	_LDB,	_STB,	// 0xf0
	_EORB,	_ADCB,	_ORB,	_ADDB,	_LDD,	_STD,	_LDU,	_STU
};

enum mnemonics_index opcodes_page_2[256] = {
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x00
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x10
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_LBRN,	_LBHI,	_LBLS,	_LBHS,	_LBLO,	_LBNE,	_LBEQ,	// 0x20
	_LBVC,	_LBVS,	_LBPL,	_LBMI,	_LBGE,	_LBLT,	_LBGT,	_LBLE,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x30
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_SWI2,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x40
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x50
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x60
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x70
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_CMPD,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x80
	_ILL,	_ILL,	_ILL,	_ILL,	_CMPY,	_ILL,	_LDY,	_ILL,
	_ILL,	_ILL,	_ILL,	_CMPD,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x90
	_ILL,	_ILL,	_ILL,	_ILL,	_CMPY,	_ILL,	_LDY,	_STY,
	_ILL,	_ILL,	_ILL,	_CMPD,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xa0
	_ILL,	_ILL,	_ILL,	_ILL,	_CMPY,	_ILL,	_LDY,	_STY,
	_ILL,	_ILL,	_ILL,	_CMPD,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xb0
	_ILL,	_ILL,	_ILL,	_ILL,	_CMPY,	_ILL,	_LDY,	_STY,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xc0
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_LDS,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xd0
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_LDS,	_STS,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xe0
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_LDS,	_STS,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xf0
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_LDS,	_STS
};

enum mnemonics_index opcodes_page_3[256] = {
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x00
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x10
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x20
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x30
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_SWI3,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x40
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x50
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x60
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x70
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_CMPU,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x80
	_ILL,	_ILL,	_ILL,	_ILL,	_CMPS,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_CMPU,	_ILL,	_ILL,	_ILL,	_ILL,	// 0x90
	_ILL,	_ILL,	_ILL,	_ILL,	_CMPS,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_CMPU,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xa0
	_ILL,	_ILL,	_ILL,	_ILL,	_CMPS,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_CMPU,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xb0
	_ILL,	_ILL,	_ILL,	_ILL,	_CMPS,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xc0
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xd0
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xe0
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	// 0xf0
	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL,	_ILL
};

uint16_t mc6809::disassemble_instruction(char *buffer, uint16_t address)
{
	char *start_buffer = buffer;
	char *mne_buffer = &buffer[21];
	uint8_t bytes_printed = 0;
	uint16_t start_address = address;
	uint8_t opcode = read_8(address++);
	buffer += sprintf(buffer, ",%04x %02x", start_address, opcode);
	bytes_printed++;

	if (opcode == 0x10) {
		// page 2
		opcode = read_8(address++);
		buffer += sprintf(buffer, " %02x", opcode);
		bytes_printed++;
		mne_buffer += sprintf(mne_buffer, " %s\n", mnemonics[opcodes_page_2[opcode]]);
	} else if (opcode == 0x11) {
		// page 3
		opcode = read_8(address++);
	} else {
		// page "1"
		mne_buffer += sprintf(mne_buffer, " %s\n", mnemonics[opcodes_page_1[opcode]]);
	}

	for (int i=0; i<(5 - bytes_printed); i++) {
		buffer += sprintf(buffer, "   ");
	}
	start_buffer[20] = ' ';
	return address - start_address;
}
