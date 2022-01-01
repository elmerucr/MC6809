/*
 * mc6809_disassembler.cpp  -  part of MC6809
 *
 * (C)2021-2022 elmerucr
 *
 * Code is inspired by dasm09 which can be found at:
 * http://koti.mbnet.fi/~atjs/mc6809/Disassembler/dasm09.TGZ
 *
 * Also by f9dasm:
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
	__DIR_,	// direct
	__NOM_,	// no mode
	__REB_,	// relative byte
	__REW_,	// relative word
	__IMB_,	// immediate byte
	__IMW_,	// immediate word
	__IBB_,	// immediate byte binary (for andcc and orcc)
	__EXT_,	// extended
	__INH_,	// inherent
	__R1_,	// tfr/exg mode
	__R2_,	// pul/psh system
	__R3_,	// pul/psh user
	__IDX_,	// indexed


	// __BD_,	// Bit Manipulation direct	// 6309??
	// __BI_,	// Bit Manipulation index	// 6309??
	// __BE_,	// Bit Manipulation extended	// 6309??
	// __BT_,	// Bit Transfers direct		// 6309??
	// __T1_,	// Block Transfer r0+,r1+	// 6309??
	// __T2_,	// Block Transfer r0-,r1-	// 6309??
	// __T3_,	// Block Transfer r0+,r1	// 6309??
	// __T4_,	// Block Transfer r0,r1+	// 6309??
	// __IML_ 	// immediate 32-bit		// 6309??
};

const char *mnemonics[133] = {
	"abx  ","adca ","adcb ","adda ","addb ","addd ","anda ","andb ",
	"andcc","asl  ","asla ","aslb ","asr  ","asra ","asrb ","beq  ",
	"bge  ","bgt  ","bhi  ","bhs  ","bita ","bitb ","bmi  ","ble  ",
	"blo  ","bls  ","blt  ","bne  ","bpl  ","bra  ","brn  ","bsr  ",
	"bvc  ","bvs  ","clr  ","clra ","clrb ","cmpa ","cmpb ","cmpd ",
	"cmps ","cmpu ","cmpx ","cmpy ","com  ","coma ","comb ","cwai ",
	"daa  ","dec  ","deca ","decb ","eora ","eorb ","exg  ","???  ",
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

struct exg_tfr_operand {
	char name[3];
	bool illegal;
	bool eight_bit;
};

const struct exg_tfr_operand exg_tfr_operands[16] = {
	{ "d",  false, false },
	{ "x",  false, false },
	{ "y",  false, false },
	{ "u",  false, false },
	{ "s",  false, false },
	{ "pc", false, false },
	{ "?",  true,  false },
	{ "?",  true,  false },
	{ "a",  false, true  },
	{ "b",  false, true  },
	{ "cc", false, true  },
	{ "dp", false, true  },
	{ "?",  true,  false },
	{ "?",  true,  false },
	{ "?",  true,  false },
	{ "?",  true,  false }
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

enum addr_mode_index addr_mode_page_1[256] = {
	__DIR_, __NOM_, __NOM_, __DIR_, __DIR_, __NOM_, __DIR_,	__DIR_,	// 0x00
	__DIR_, __DIR_, __DIR_, __NOM_, __DIR_, __DIR_, __NOM_, __DIR_,
	__NOM_, __NOM_, __INH_, __INH_, __NOM_, __NOM_, __REW_, __REW_,	// 0x10
	__NOM_, __INH_, __IBB_, __NOM_, __IBB_, __INH_, __R1_, 	__R1_,
	__REB_, __REB_, __REB_, __REB_, __REB_, __REB_, __REB_, __REB_,	// 0x20
	__REB_, __REB_, __REB_, __REB_, __REB_, __REB_, __REB_, __REB_,
	__IDX_, __IDX_, __IDX_, __IDX_, __R2_,  __R2_,  __R3_,  __R3_,	// 0x30
	__NOM_, __INH_, __INH_, __INH_, __IMB_, __INH_, __NOM_, __INH_,
	__INH_, __NOM_, __NOM_, __INH_, __INH_, __NOM_, __INH_, __INH_,	// 0x40
	__INH_, __INH_, __INH_, __NOM_, __INH_, __INH_, __NOM_, __INH_,
	__INH_, __NOM_, __NOM_, __INH_, __INH_, __NOM_, __INH_, __INH_,	// 0x50
	__INH_, __INH_, __INH_, __NOM_, __INH_, __INH_, __NOM_, __INH_,
	__IDX_, __NOM_, __NOM_, __IDX_, __IDX_, __NOM_, __IDX_, __IDX_,	// 0x60
	__IDX_, __IDX_, __IDX_, __NOM_, __IDX_, __IDX_, __IDX_, __IDX_,
	__EXT_, __NOM_, __NOM_, __EXT_, __EXT_, __NOM_, __EXT_, __EXT_,	// 0x70
	__EXT_, __EXT_, __EXT_, __NOM_, __EXT_, __EXT_, __EXT_, __EXT_,
	__IMB_, __IMB_, __IMB_, __IMW_, __IMB_, __IMB_, __IMB_, __NOM_,	// 0x80
	__IMB_, __IMB_, __IMB_, __IMB_, __IMW_, __REB_, __IMW_, __NOM_,
	__DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_,	// 0x90
	__DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_,
	__IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_,	// 0xa0
	__IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_,
	__EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_,	// 0xb0
	__EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_,
	__IMB_, __IMB_, __IMB_, __IMW_, __IMB_, __IMB_, __IMB_, __NOM_,	// 0xc0
	__IMB_, __IMB_, __IMB_, __IMB_, __IMW_, __NOM_, __IMW_, __NOM_,
	__DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_,	// 0xd0
	__DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_, __DIR_,
	__IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_,	// 0xe0
	__IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_, __IDX_,
	__EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_,	// 0xf0
	__EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_, __EXT_
};

enum addr_mode_index addr_mode_page_2[256] = {
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x00
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x10
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __REW_, __REW_, __REW_, __REW_, __REW_, __REW_, __REW_,	// 0x20
	__REW_, __REW_, __REW_, __REW_, __REW_, __REW_, __REW_, __REW_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x30
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __INH_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x40
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x50
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x60
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x70
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __IMW_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x80
	__NOM_, __NOM_, __NOM_, __NOM_, __IMW_, __NOM_, __IMW_, __NOM_,
	__NOM_, __NOM_, __NOM_, __DIR_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x90
	__NOM_, __NOM_, __NOM_, __NOM_, __DIR_, __NOM_, __DIR_, __DIR_,
	__NOM_, __NOM_, __NOM_, __IDX_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xa0
	__NOM_, __NOM_, __NOM_, __NOM_, __IDX_, __NOM_, __IDX_, __IDX_,
	__NOM_, __NOM_, __NOM_, __EXT_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xb0
	__NOM_, __NOM_, __NOM_, __NOM_, __EXT_, __NOM_, __EXT_, __EXT_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xc0
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __IMW_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xd0
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __DIR_, __DIR_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xe0
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __IDX_, __IDX_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xf0
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __EXT_, __EXT_
};

enum addr_mode_index addr_mode_page_3[256] = {
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x00
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x10
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x20
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x30
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __INH_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x40
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x50
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x60
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x70
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __IMW_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x80
	__NOM_, __NOM_, __NOM_, __NOM_, __IMW_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __DIR_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0x90
	__NOM_, __NOM_, __NOM_, __NOM_, __DIR_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __IDX_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xa0
	__NOM_, __NOM_, __NOM_, __NOM_, __IDX_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __EXT_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xb0
	__NOM_, __NOM_, __NOM_, __NOM_, __EXT_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xc0
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xd0
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xe0
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_,	// 0xf0
	__NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_, __NOM_
};

uint16_t mc6809::disassemble_instruction(char *buffer, uint16_t address)
{
	disassemble_success = true;

	const char *idx_reg_names[4] = {
		"x", "y", "u", "s"
	};

	/* pul/psh system register names */
	const char *r2_reg_names[8] = {
		"pc", "u", "y", "x", "dp", "b", "a", "cc"
	};

	/* pul/psh user register names */
	const char *r3_reg_names[8] = {
		"pc", "s", "y", "x", "dp", "b", "a", "cc"
	};

	const char *offset_5_bit[32] = {
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
		"11", "12", "13", "14", "15", "-16", "-15", "-14", "-13",
		"-12", "-11", "-10", "-9", "-8", "-7", "-6", "-5", "-4",
		"-3", "-2", "-1"
	};

	/*
	 * original_buffer points to the original start. Now it is
	 * possible to refer to a fixed char (and remove the \0 )
	 */
	char *original_buffer = buffer;

	/*
	 * mnemonics buffer = starting point of the mnemonic in the
	 * disassembler output.
	 */
	char *mne_buffer = &buffer[17];

	/*
	 * Any instruction has a maximum of 5 bytes - keep track of
	 * the number of printed bytes, to know how many spaces need
	 * to be printed.
	 */
	uint8_t bytes_printed = 0;

	/*
	 * Keep track of the original start address, to calculate the
	 * total number of bytes that the instruction takes.
	 */
	uint16_t start_address = address;

	enum addr_mode_index mode;

	uint8_t byte = (*read_8)(address++);
	uint8_t byte2 = 0;
	uint16_t word = 0;
	buffer += sprintf(buffer, ",%04x %02x", start_address, byte);
	bytes_printed++;

	if (byte == 0x10) {
		// page 2
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		mne_buffer += sprintf(mne_buffer, "%s ",
			mnemonics[opcodes_page_2[byte]]);
		mode = addr_mode_page_2[byte];
	} else if (byte == 0x11) {
		// page 3
		byte = read_8(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		mne_buffer += sprintf(mne_buffer, "%s ",
			mnemonics[opcodes_page_3[byte]]);
		mode = addr_mode_page_3[byte];
	} else {
		// page "1"
		mne_buffer += sprintf(mne_buffer, "%s ",
			mnemonics[opcodes_page_1[byte]]);
		mode = addr_mode_page_1[byte];
	}

	if (mode == __NOM_) disassemble_success = false;

	switch (mode) {
	case __DIR_:
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		mne_buffer += sprintf(mne_buffer,
			"$%02x", byte);
		break;
	case __REB_:
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		mne_buffer += sprintf(mne_buffer, "$%04x",
			(uint16_t)(address +
			(uint16_t)((int8_t)byte)));
		break;
	case __REW_:
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		word = byte << 8;
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		word |= byte;
		mne_buffer += sprintf(mne_buffer, "$%04x",
			(uint16_t)(address + word));
		break;
	case __IMB_:
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		mne_buffer += sprintf(mne_buffer,
			"#$%02x", byte);
		break;
	case __IMW_:
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		mne_buffer += sprintf(mne_buffer,
			"#$%02x", byte);
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		mne_buffer += sprintf(mne_buffer,
			"%02x", byte);
		break;
	case __IBB_:
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		mne_buffer += sprintf(mne_buffer,
			"#%%%c%c%c%c%c%c%c%c",
			byte & 0x80 ? '1' : '0',
			byte & 0x40 ? '1' : '0',
			byte & 0x20 ? '1' : '0',
			byte & 0x10 ? '1' : '0',
			byte & 0x08 ? '1' : '0',
			byte & 0x04 ? '1' : '0',
			byte & 0x02 ? '1' : '0',
			byte & 0x01 ? '1' : '0');
		break;
	case __EXT_:
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		word = byte << 8;
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		word |= byte;
		mne_buffer += sprintf(mne_buffer,
			"$%04x", word);
		break;
	case __IDX_:
		// read postbyte
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		if (byte == 0b10011111) {
			// indirect extended
			mne_buffer += sprintf(mne_buffer, "[");
			byte = (*read_8)(address++);
			buffer += sprintf(buffer, "%02x", byte);
			bytes_printed++;
			word = byte << 8;
			byte = (*read_8)(address++);
			buffer += sprintf(buffer, "%02x", byte);
			bytes_printed++;
			word |= byte;
			mne_buffer += sprintf(mne_buffer, "$%04x]", word);
		} else {
			switch (byte & 0b10000000) {
			case 0b10000000:
				switch (byte & 0b00010000) {
				case 0b00000000:
					// non-indirect
					switch (byte & 0b00001111) {
					case 0b0100:
						// no offset
						mne_buffer += sprintf(mne_buffer,
							",%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1000:
						// 8 bit offset
						byte2 = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte2);
						bytes_printed++;
						mne_buffer += sprintf(mne_buffer,
							"$%02x,%s",
							byte2,
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1001:
						// 16 bit offset
						byte2 = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte2);
						bytes_printed++;
						word = byte2 << 8;
						byte2 = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte2);
						bytes_printed++;
						word |= byte2;
						mne_buffer += sprintf(mne_buffer,
							"$%04x,%s",
							word,
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0110:
						// accu a offset
						mne_buffer += sprintf(mne_buffer,
							"a,%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0101:
						// accu b offset
						mne_buffer += sprintf(mne_buffer,
							"b,%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1011:
						// accu d offset
						mne_buffer += sprintf(mne_buffer,
							"d,%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0000:
						// auto increment by 1
						mne_buffer += sprintf(mne_buffer,
							",%s+",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0001:
						// auto increment by 2
						mne_buffer += sprintf(mne_buffer,
							",%s++",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0010:
						// auto decrement by 1
						mne_buffer += sprintf(mne_buffer,
							",-%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0011:
						// auto decrement by 2
						mne_buffer += sprintf(mne_buffer,
							",--%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1100:
						// const offset pc 8bit, read extra byte
						byte = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte);
						bytes_printed++;
						mne_buffer += sprintf(mne_buffer,
							"$%02x,pc",
							byte);
						break;
					case 0b1101:
						// const offs pc 16 bit, read 2 extr bytes
						byte = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte);
						bytes_printed++;
						word = byte << 8;
						byte = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte);
						bytes_printed++;
						word |= byte;
						mne_buffer += sprintf(mne_buffer,
							"$%04x,pc",
							word);
						break;
					default:
						// all others are illegal
						mne_buffer += sprintf(mne_buffer,
							"illegal");
						disassemble_success = false;
						break;
					}
					break;
				case 0b00010000:
					// indirect
					switch (byte & 0b00001111) {
					case 0b0100:
						// indirect no offset
						mne_buffer += sprintf(mne_buffer,
							"[,%s]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1000:
						// indirect 8 bit offset
						byte2 = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte2);
						bytes_printed++;
						mne_buffer += sprintf(mne_buffer,
							"[$%02x,%s]",
							byte2,
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1001:
						// indirect 16 bit offset
						byte2 = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte2);
						bytes_printed++;
						word = byte2 << 8;
						byte2 = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte2);
						bytes_printed++;
						word |= byte2;
						mne_buffer += sprintf(mne_buffer,
							"[$%04x,%s]",
							word,
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0110:
						// indirect accu a offset
						mne_buffer += sprintf(mne_buffer,
							"[a,%s]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0101:
						// indirect accu b offset
						mne_buffer += sprintf(mne_buffer,
							"[b,%s]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1011:
						// indirect accu d offset
						mne_buffer += sprintf(mne_buffer,
							"[d,%s]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0001:
						// indirect auto increment by 2
						mne_buffer += sprintf(mne_buffer,
							"[,%s++]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0011:
						// indirect auto decrement by 2
						mne_buffer += sprintf(mne_buffer,
							"[,--%s]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1100:
						// indirect const offset pc 8bit, read extra byte
						byte = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte);
						bytes_printed++;
						mne_buffer += sprintf(mne_buffer,
							"[$%02x,pc]",
							byte);
						break;
					case 0b1101:
						// indirect const offs pc 16 bit, read 2 extr bytes
						byte = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte);
						bytes_printed++;
						word = byte << 8;
						byte = (*read_8)(address++);
						buffer += sprintf(buffer, "%02x", byte);
						bytes_printed++;
						word |= byte;
						mne_buffer += sprintf(mne_buffer,
							"[$%04x,pc]",
							word);
						break;
					default:
						// all others are illegal
						mne_buffer += sprintf(mne_buffer,
							"illegal");
						disassemble_success = false;
						break;
					}
					break;
				}
				break;
			case 0b00000000:
				// non-indirect
				// constant 5 bit signed offset
				mne_buffer += sprintf(mne_buffer, "%s,%s",
					offset_5_bit[byte & 0b00011111],
					idx_reg_names[(byte & 0b01100000) >> 5]);
				break;
			default:
				break;
			}
		}
		break;
	case __R1_:
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		if (((exg_tfr_operands[byte >> 4].illegal) || (exg_tfr_operands[byte & 0x0f].illegal)) ||
		((exg_tfr_operands[byte >> 4].eight_bit) != (exg_tfr_operands[byte & 0x0f].eight_bit))) {
			mne_buffer += sprintf(mne_buffer, "illegal");
			disassemble_success = false;
		} else {
			mne_buffer += sprintf(mne_buffer, "%s,%s",
				exg_tfr_operands[(byte >> 4)].name,
				exg_tfr_operands[byte & 0x0f].name);
		}
		break;
	case __R2_:
		// pul/psh system
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		if (byte == 0x00) {
			mne_buffer += sprintf(mne_buffer, "<empty>");
		} else {
			int number = 0;
			for (uint8_t i=0x80; i != 0; i /= 2) {
				if (byte & i) {
					mne_buffer += sprintf(
						mne_buffer,
						"%s,",
						r2_reg_names[number]);
				}
				number++;
			}
			mne_buffer--;		// remove last komma
			*mne_buffer = '\0';
		}
		break;
	case __R3_:
		// pul/psh user
		byte = (*read_8)(address++);
		buffer += sprintf(buffer, "%02x", byte);
		bytes_printed++;
		if (byte == 0x00) {
			mne_buffer += sprintf(mne_buffer, "<empty>");
		} else {
			int number = 0;
			for (uint8_t i=0x80; i != 0; i /= 2) {
				if (byte & i) {
					mne_buffer += sprintf(
						mne_buffer,
						"%s,",
						r3_reg_names[number]);
				}
				number++;
			}
			mne_buffer--;		// remove last komma
			*mne_buffer = '\0';
		}
		break;
	case __INH_:
		// no further information to be printed
		break;
	case __NOM_:
		break;
	default:
		break;
	};

	for (int i=0; i<(5 - bytes_printed); i++) {
		buffer += sprintf(buffer, "  ");
	}
	original_buffer[16] = ' ';
	return address - start_address;
}
