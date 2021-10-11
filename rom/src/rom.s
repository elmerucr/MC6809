	global	vector_reset
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

vector_swi2:

vector_firq:

vector_irq:

vector_swi:

vector_nmi:

vector_reset:
	lds	#$2000