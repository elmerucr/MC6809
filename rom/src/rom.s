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
	sex
	ldx	#vector_reset
	rti

vector_swi2:

vector_firq:

vector_irq:

vector_swi:

vector_nmi:

vector_reset:
	ldx	#$2000
	ldb	[$45,x]
	ldb	.1,pc
	lbra	vector_reset
.1	db	$2e, $2f, $01
