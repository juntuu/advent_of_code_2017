.text
.align	2

; Solve:
;  parameter:
;  - w0: the value of register a
;  return:
;  - w0: number of mul instructions
;  - w1: the final value of register h
; Register mapping:
;   a -> w0 (parameter)
;   b -> x8
;   c -> x9
;   d -> x10
;   e -> x11
;   f -> x12
;   g -> x13
;   h -> w1 (return value)
;   temp -> x14..x18
; Instruction mapping:
;  set -> mov
;  mul -> mul
;  sub -> sub
;  jnz -> cbnz (offset multiplied by 4, or replaced by label when constant)
;  jnz -> b (when non-zero constant condition)

solve:
	; clear the return register
	mov	w1, #0
	; set required constants
	mov	x16, #-1000
	mov	x15, #17
	mov	x14, #100      ; #100
	mul	x15, x15, x16  ; #-17000
	mul	x16, x16, x14  ; #-100000

	mov	x8, #65
	mov	x9, x8
	cbnz	w0, part2
	b	main_loop
part2:
	mov	w0, #1  ; reset to 1 accounting for the following mul
	mul	x8, x8, x14
	sub	x8, x8, x16
	sub	x9, x8, x15
main_loop:
	; calculate #mul instructions in inner loops
	sub	x14, x8, #2
	madd	x0, x14, x14, x0

	mov	x10, #2
loop1:  ; x10 = 2..x8
	sdiv	x13, x8, x10       ; a = âx / bâ
	msub	x13, x13, x10, x8  ; x - (a * b) = 0 â x % b = 0
	cbnz	x13, else
	add	w1, w1, #1
	b	break
else:
	add	x10, x10, #1
	sub	x13, x10, x8
	cbnz	x13, loop1
break:
	sub	x13, x8, x9
	add	x8, x8, #17
	cbnz	x13, main_loop
	ret

.global	_main
_main:
	stp	x29, x30, [sp, #-16]!
	mov	x29, sp

	; Part 1
	mov	w0, #0  ; set a to 0
	bl	solve   ; #mul instructions returned in w0
	bl	print

	; Part 2
	mov	w0, #1  ; set a to 1
	bl	solve   ; value in register h returned in w1
	mov	w0, w1  ; move w1 to w0 for printing
	bl	print

	mov	w0, #0
	ldp	x29, x30, [sp], #16
	ret

; Print integer in w0
print:
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]
	add	x29, sp, #16

	str	w0, [sp]
	adrp	x0, format@PAGE
	add	x0, x0, format@PAGEOFF
	bl	_printf

	ldp	x29, x30, [sp, #16]
	add	sp, sp, #32
	ret

format:
	.asciz	"%d\n"
