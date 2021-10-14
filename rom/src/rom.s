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

	ldb	#7
	ldx	#data
	ldy	#$0041
.1	lda	,x+
	sta	,y+
	decb
	bne	.1

sort:	lda	#1
	sta	$40
	lda	$41
	deca
	ldx	#$42
pass:	ldb	,x+	; is pair of elements in order
	cmpb	,x
	bhs	count
	clr	$40
	pshs	a
	lda	,x
	stb	,x
	sta	-1,x
	puls	a
count:	deca
	bne	pass
	tst	$40
	beq	sort
	swi



test:
	ldd	#$ffff
	cmpd	#$ffff
	rts

data:
	db	$06	; length of array
	db	$2a, $b5, $60, $3f, $d1, $19