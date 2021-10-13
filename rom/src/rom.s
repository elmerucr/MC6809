RAM1	equ	$2000

	global	vector_illegal_opcode
	global	vector_swi3
	global	vector_swi2
	global	vector_firq
	global	vector_irq
	global	vector_swi
	global	vector_nmi
	global	vector_reset

	section	TEXT

vector_illegal_opcode:

vector_swi3:
	abx
	sex
	ldx	#vector_reset
	rti

vector_swi2:

vector_firq:

vector_irq:

vector_swi:

vector_nmi:

vector_reset:
	; set stackpointers
	lds	#$1000
	ldu	#$0800

.1	lbsr	test
	ldx	#$1234
	ldy	#$5678
	pshu	x,y
	ldx	#$ffff
	ldy	#$eeee
	pulu	x,y
	lbra	.1

test:
	lda	#$05
	cmpa	#$06

.1	asrb
	bne	.1
	rts
