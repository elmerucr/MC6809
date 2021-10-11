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
	sex
	ldx	#vector_reset
	rti

vector_swi2:

vector_firq:

vector_irq:

vector_swi:

vector_nmi:

vector_reset:
	orcc	#%10000001
	lda	[$2000]
	andcc	#%11100111
	; orcc	#%00000100
	pshs	x,y,pc,u
	bne	vector_reset