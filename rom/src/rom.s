RAM1	equ	$2000

	global	vector_illop
	global	vector_swi3
	global	vector_swi2
	global	vector_firq
	global	vector_irq
	global	vector_swi
	global	vector_nmi
	global	vector_reset

	section	TEXT

vector_illop:
	rti

vector_swi3:
	rti

vector_swi2:

vector_firq:
	ldd	#$dead
	ldd	#$beef
	rti

vector_irq:
	ldd	#$dead
	ldd	#$beef
	rti

vector_swi:
	rti

vector_nmi:
	coma
	rti

vector_reset:
	; set stackpointers
	lds	#$1000
	ldu	#$0800

	; enable firq's
	andcc	#%10101111

	ldd	#$fffd
.1	cmpd	#$0002
	bgt	.2
	addd	#$0001
	bra	.1
.2	ldx	#$dead
	ldy	#$beef
