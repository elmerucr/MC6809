/*
 * mc6809_disassembler.cpp  -  part of MC6809
 *
 * (C)2021-2025 elmerucr
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
	__ABX=0,__ADCA, __ADCB, __ADDA, __ADDB, __ADDD, __ANDA, __ANDB,
	__ANDCC,__ASL,  __ASLA, __ASLB, __ASR,  __ASRA, __ASRB, __BEQ,
	__BGE,  __BGT,  __BHI,  __BHS,  __BITA, __BITB, __BMI,  __BLE,
	__BLO,  __BLS,  __BLT,  __BNE,  __BPL,  __BRA,  __BRN,  __BSR,
	__BVC,  __BVS,  __CLR,  __CLRA, __CLRB, __CMPA, __CMPB, __CMPD,
	__CMPS, __CMPU, __CMPX, __CMPY, __COM,  __COMA, __COMB, __CWAI,
	__DAA,  __DEC,  __DECA, __DECB, __EORA, __EORB, __EXG,  __ILL,
	__INC,  __INCA, __INCB, __JMP,  __JSR,  __LBEQ, __LBGE, __LBGT,
	__LBHI, __LBHS, __LBLE, __LBLO, __LBLS, __LBLT, __LBMI, __LBNE,
	__LBPL, __LBRA, __LBRN, __LBSR, __LBVC, __LBVS, __LDA,  __LDB,
	__LDD,  __LDS,  __LDU,  __LDX,  __LDY,  __LEAS, __LEAU, __LEAX,
	__LEAY, __LSR,  __LSRA, __LSRB, __MUL,  __NEG,  __NEGA, __NEGB,
	__NOP,  __ORA,  __ORB,  __ORCC, __PSHS, __PSHU, __PULS, __PULU,
	__ROL,  __ROLA, __ROLB, __ROR,  __RORA, __RORB, __RTI,  __RTS,
	__SBCA, __SBCB, __SEX,  __STA,  __STB,  __STD,  __STS,  __STU,
	__STX,  __STY,  __SUBA, __SUBB, __SUBD, __SWI,  __SWI2, __SWI3,
	__SYNC, __TFR,  __TST,  __TSTA, __TSTB
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
	__NEG,	__ILL,	__ILL,	__COM,	__LSR,	__ILL,	__ROR,	__ASR,	// 0x00
	__ASL,	__ROL,	__DEC,	__ILL,	__INC,	__TST,	__JMP,	__CLR,
	__ILL,	__ILL,	__NOP,	__SYNC,	__ILL,	__ILL,	__LBRA,	__LBSR,	// 0x10
	__ILL,	__DAA,	__ORCC,	__ILL,	__ANDCC,__SEX,	__EXG,	__TFR,
	__BRA,	__BRN,	__BHI,	__BLS,	__BHS,	__BLO,	__BNE,	__BEQ,	// 0x20
	__BVC,	__BVS,	__BPL,	__BMI,	__BGE,	__BLT,	__BGT,	__BLE,
	__LEAX,	__LEAY,	__LEAS,	__LEAU,	__PSHS,	__PULS,	__PSHU,	__PULU,	// 0x30
	__ILL,	__RTS,	__ABX,	__RTI,	__CWAI,	__MUL,	__ILL,	__SWI,
	__NEGA,	__ILL,	__ILL,	__COMA,	__LSRA,	__ILL,	__RORA,	__ASRA,	// 0x40
	__ASLA,	__ROLA,	__DECA,	__ILL,	__INCA,	__TSTA,	__ILL,	__CLRA,
	__NEGB,	__ILL,	__ILL,	__COMB,	__LSRB,	__ILL,	__RORB,	__ASRB,	// 0x50
	__ASLB,	__ROLB,	__DECB,	__ILL,	__INCB,	__TSTB,	__ILL,	__CLRB,
	__NEG,	__ILL,	__ILL,	__COM,	__LSR,	__ILL,	__ROR,	__ASR,	// 0x60
	__ASL,	__ROL,	__DEC,	__ILL,	__INC,	__TST,	__JMP,	__CLR,
	__NEG,	__ILL,	__ILL,	__COM,	__LSR,	__ILL,	__ROR,	__ASR,	// 0x70
	__ASL,	__ROL,	__DEC,	__ILL,	__INC,	__TST,	__JMP,	__CLR,
	__SUBA,	__CMPA,	__SBCA,	__SUBD,	__ANDA,	__BITA,	__LDA,	__ILL,	// 0x80
	__EORA,	__ADCA,	__ORA,	__ADDA,	__CMPX,	__BSR,	__LDX,	__ILL,
	__SUBA,	__CMPA,	__SBCA,	__SUBD,	__ANDA,	__BITA,	__LDA,	__STA,	// 0x90
	__EORA,	__ADCA,	__ORA,	__ADDA,	__CMPX,	__JSR,	__LDX,	__STX,
	__SUBA,	__CMPA,	__SBCA,	__SUBD,	__ANDA,	__BITA,	__LDA,	__STA,	// 0xa0
	__EORA,	__ADCA,	__ORA,	__ADDA,	__CMPX,	__JSR,	__LDX,	__STX,
	__SUBA,	__CMPA,	__SBCA,	__SUBD,	__ANDA,	__BITA,	__LDA,	__STA,	// 0xb0
	__EORA,	__ADCA,	__ORA,	__ADDA,	__CMPX,	__JSR,	__LDX,	__STX,
	__SUBB,	__CMPB,	__SBCB,	__ADDD,	__ANDB,	__BITB,	__LDB,	__ILL,	// 0xc0
	__EORB,	__ADCB,	__ORB,	__ADDB,	__LDD,	__ILL,	__LDU,	__ILL,
	__SUBB,	__CMPB,	__SBCB,	__ADDD,	__ANDB,	__BITB,	__LDB,	__STB,	// 0xd0
	__EORB,	__ADCB,	__ORB,	__ADDB,	__LDD,	__STD,	__LDU,	__STU,
	__SUBB,	__CMPB,	__SBCB,	__ADDD,	__ANDB,	__BITB,	__LDB,	__STB,	// 0xe0
	__EORB,	__ADCB,	__ORB,	__ADDB,	__LDD,	__STD,	__LDU,	__STU,
	__SUBB,	__CMPB,	__SBCB,	__ADDD,	__ANDB,	__BITB,	__LDB,	__STB,	// 0xf0
	__EORB,	__ADCB,	__ORB,	__ADDB,	__LDD,	__STD,	__LDU,	__STU
};

enum mnemonics_index opcodes_page_2[256] = {
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x00
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x10
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__LBRN,	__LBHI,	__LBLS,	__LBHS,	__LBLO,	__LBNE,	__LBEQ,	// 0x20
	__LBVC,	__LBVS,	__LBPL,	__LBMI,	__LBGE,	__LBLT,	__LBGT,	__LBLE,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x30
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__SWI2,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x40
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x50
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x60
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x70
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__CMPD,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x80
	__ILL,	__ILL,	__ILL,	__ILL,	__CMPY,	__ILL,	__LDY,	__ILL,
	__ILL,	__ILL,	__ILL,	__CMPD,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x90
	__ILL,	__ILL,	__ILL,	__ILL,	__CMPY,	__ILL,	__LDY,	__STY,
	__ILL,	__ILL,	__ILL,	__CMPD,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xa0
	__ILL,	__ILL,	__ILL,	__ILL,	__CMPY,	__ILL,	__LDY,	__STY,
	__ILL,	__ILL,	__ILL,	__CMPD,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xb0
	__ILL,	__ILL,	__ILL,	__ILL,	__CMPY,	__ILL,	__LDY,	__STY,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xc0
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__LDS,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xd0
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__LDS,	__STS,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xe0
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__LDS,	__STS,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xf0
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__LDS,	__STS
};

enum mnemonics_index opcodes_page_3[256] = {
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x00
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x10
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x20
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x30
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__SWI3,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x40
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x50
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x60
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x70
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__CMPU,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x80
	__ILL,	__ILL,	__ILL,	__ILL,	__CMPS,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__CMPU,	__ILL,	__ILL,	__ILL,	__ILL,	// 0x90
	__ILL,	__ILL,	__ILL,	__ILL,	__CMPS,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__CMPU,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xa0
	__ILL,	__ILL,	__ILL,	__ILL,	__CMPS,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__CMPU,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xb0
	__ILL,	__ILL,	__ILL,	__ILL,	__CMPS,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xc0
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xd0
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xe0
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	// 0xf0
	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL,	__ILL
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

uint16_t mc6809::disassemble_instruction(char *buffer, size_t n, uint16_t address)
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

	uint8_t byte = read8(address++);
	uint8_t byte2 = 0;
	uint16_t word = 0;
	buffer += snprintf(buffer, n, ",%04x %02x", start_address, byte);
	bytes_printed++;

	if (byte == 0x10) {
		// page 2
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		mne_buffer += snprintf(mne_buffer, 17, "%s ",
			mnemonics[opcodes_page_2[byte]]);
		mode = addr_mode_page_2[byte];
	} else if (byte == 0x11) {
		// page 3
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		mne_buffer += snprintf(mne_buffer, 17, "%s ",
			mnemonics[opcodes_page_3[byte]]);
		mode = addr_mode_page_3[byte];
	} else {
		// page "1"
		mne_buffer += snprintf(mne_buffer, 17, "%s ",
			mnemonics[opcodes_page_1[byte]]);
		mode = addr_mode_page_1[byte];
	}

	if (mode == __NOM_) disassemble_success = false;

	switch (mode) {
	case __DIR_:
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		mne_buffer += snprintf(mne_buffer, 17,
			"$%02x", byte);
		break;
	case __REB_:
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		mne_buffer += snprintf(mne_buffer, 17, "$%04x",
			(uint16_t)(address +
			(uint16_t)((int8_t)byte)));
		break;
	case __REW_:
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		word = byte << 8;
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		word |= byte;
		mne_buffer += snprintf(mne_buffer, 17, "$%04x",
			(uint16_t)(address + word));
		break;
	case __IMB_:
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		mne_buffer += snprintf(mne_buffer, 17,
			"#$%02x", byte);
		break;
	case __IMW_:
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		mne_buffer += snprintf(mne_buffer, 17,
			"#$%02x", byte);
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		mne_buffer += snprintf(mne_buffer, 17,
			"%02x", byte);
		break;
	case __IBB_:
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		mne_buffer += snprintf(mne_buffer, 17,
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
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		word = byte << 8;
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		word |= byte;
		mne_buffer += snprintf(mne_buffer, 17,
			"$%04x", word);
		break;
	case __IDX_:
		// read postbyte
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		if (byte == 0b10011111) {
			// indirect extended
			mne_buffer += snprintf(mne_buffer, 17, "[");
			byte = read8(address++);
			buffer += snprintf(buffer, n, "%02x", byte);
			bytes_printed++;
			word = byte << 8;
			byte = read8(address++);
			buffer += snprintf(buffer, n, "%02x", byte);
			bytes_printed++;
			word |= byte;
			mne_buffer += snprintf(mne_buffer, 17, "$%04x]", word);
		} else {
			switch (byte & 0b10000000) {
			case 0b10000000:
				switch (byte & 0b00010000) {
				case 0b00000000:
					// non-indirect
					switch (byte & 0b00001111) {
					case 0b0100:
						// no offset
						mne_buffer += snprintf(mne_buffer, 17,
							",%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1000:
						// 8 bit offset
						byte2 = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte2);
						bytes_printed++;
						mne_buffer += snprintf(mne_buffer, 17,
							"$%02x,%s",
							byte2,
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1001:
						// 16 bit offset
						byte2 = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte2);
						bytes_printed++;
						word = byte2 << 8;
						byte2 = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte2);
						bytes_printed++;
						word |= byte2;
						mne_buffer += snprintf(mne_buffer, 17,
							"$%04x,%s",
							word,
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0110:
						// accu a offset
						mne_buffer += snprintf(mne_buffer, 17,
							"a,%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0101:
						// accu b offset
						mne_buffer += snprintf(mne_buffer, 17,
							"b,%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1011:
						// accu d offset
						mne_buffer += snprintf(mne_buffer, 17,
							"d,%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0000:
						// auto increment by 1
						mne_buffer += snprintf(mne_buffer, 17,
							",%s+",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0001:
						// auto increment by 2
						mne_buffer += snprintf(mne_buffer, 17,
							",%s++",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0010:
						// auto decrement by 1
						mne_buffer += snprintf(mne_buffer, 17,
							",-%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0011:
						// auto decrement by 2
						mne_buffer += snprintf(mne_buffer, 17,
							",--%s",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1100:
						// const offset pc 8bit, read extra byte
						byte = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte);
						bytes_printed++;
						mne_buffer += snprintf(mne_buffer, 17,
							"$%02x,pc",
							byte);
						break;
					case 0b1101:
						// const offs pc 16 bit, read 2 extr bytes
						byte = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte);
						bytes_printed++;
						word = byte << 8;
						byte = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte);
						bytes_printed++;
						word |= byte;
						mne_buffer += snprintf(mne_buffer, 17,
							"$%04x,pc",
							word);
						break;
					default:
						// all others are illegal
						mne_buffer += snprintf(mne_buffer, 17,
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
						mne_buffer += snprintf(mne_buffer, 17,
							"[,%s]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1000:
						// indirect 8 bit offset
						byte2 = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte2);
						bytes_printed++;
						mne_buffer += snprintf(mne_buffer, 17,
							"[$%02x,%s]",
							byte2,
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1001:
						// indirect 16 bit offset
						byte2 = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte2);
						bytes_printed++;
						word = byte2 << 8;
						byte2 = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte2);
						bytes_printed++;
						word |= byte2;
						mne_buffer += snprintf(mne_buffer, 17,
							"[$%04x,%s]",
							word,
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0110:
						// indirect accu a offset
						mne_buffer += snprintf(mne_buffer, 17,
							"[a,%s]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0101:
						// indirect accu b offset
						mne_buffer += snprintf(mne_buffer, 17,
							"[b,%s]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1011:
						// indirect accu d offset
						mne_buffer += snprintf(mne_buffer, 17,
							"[d,%s]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0001:
						// indirect auto increment by 2
						mne_buffer += snprintf(mne_buffer, 17,
							"[,%s++]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b0011:
						// indirect auto decrement by 2
						mne_buffer += snprintf(mne_buffer, 17,
							"[,--%s]",
							idx_reg_names[(byte & 0b01100000) >> 5]);
						break;
					case 0b1100:
						// indirect const offset pc 8bit, read extra byte
						byte = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte);
						bytes_printed++;
						mne_buffer += snprintf(mne_buffer, 17,
							"[$%02x,pc]",
							byte);
						break;
					case 0b1101:
						// indirect const offs pc 16 bit, read 2 extr bytes
						byte = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte);
						bytes_printed++;
						word = byte << 8;
						byte = read8(address++);
						buffer += snprintf(buffer, n, "%02x", byte);
						bytes_printed++;
						word |= byte;
						mne_buffer += snprintf(mne_buffer, 17,
							"[$%04x,pc]",
							word);
						break;
					default:
						// all others are illegal
						mne_buffer += snprintf(mne_buffer, 17,
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
				mne_buffer += snprintf(mne_buffer, 17, "%s,%s",
					offset_5_bit[byte & 0b00011111],
					idx_reg_names[(byte & 0b01100000) >> 5]);
				break;
			default:
				break;
			}
		}
		break;
	case __R1_:
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		if (((exg_tfr_operands[byte >> 4].illegal) || (exg_tfr_operands[byte & 0x0f].illegal)) ||
		((exg_tfr_operands[byte >> 4].eight_bit) != (exg_tfr_operands[byte & 0x0f].eight_bit))) {
			mne_buffer += snprintf(mne_buffer, 17, "illegal");
			disassemble_success = false;
		} else {
			mne_buffer += snprintf(mne_buffer, 17, "%s,%s",
				exg_tfr_operands[(byte >> 4)].name,
				exg_tfr_operands[byte & 0x0f].name);
		}
		break;
	case __R2_:
		// pul/psh system
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		if (byte == 0x00) {
			mne_buffer += snprintf(mne_buffer, 17, "<empty>");
		} else {
			int number = 0;
			for (uint8_t i=0x80; i != 0; i /= 2) {
				if (byte & i) {
					mne_buffer += snprintf(
						mne_buffer,
						n,
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
		byte = read8(address++);
		buffer += snprintf(buffer, n, "%02x", byte);
		bytes_printed++;
		if (byte == 0x00) {
			mne_buffer += snprintf(mne_buffer, 17, "<empty>");
		} else {
			int number = 0;
			for (uint8_t i=0x80; i != 0; i /= 2) {
				if (byte & i) {
					mne_buffer += snprintf(
						mne_buffer,
						n,
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
		buffer += snprintf(buffer, n, "  ");
	}
	original_buffer[16] = ' ';
	return address - start_address;
}
