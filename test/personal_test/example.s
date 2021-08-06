.arch armv7
.macro mov32, reg, val
	movw \reg, #:lower16:\val
	movt \reg, #:upper16:\val
.endm
.data
.global a
a:
	.word	1
	.word	2
	.word	3
	.word	4
	.space	24
	.word	1
	.word	2
	.word	3
	.space	348
.data
.global b
b:
	.word	10
.data
.global c
c:
	.word	0
.data
.global d
d:
	.space	21168
.data
.global e
e:
	.space	5152
.data
.global s
s:
	.word	2
.text
.global	func2
.type	func2, %function
.syntax unified
.thumb
.thumb_func
func2:
	push	{r4-r6, r10, fp, lr}
	sub	sp, sp, #0
	mov	r4, r0
	mov	r0, r1
	mov	r0, r2
	mov	r0, r3
	mov	r10, #10
	mov	r0, r10
	mov	fp, #10
	add	r1, r4, fp
	mov	r10, #11
	mov32	fp, #1254
	add	r5, r10, fp
.L0:
	mov	fp, #0
	cmp	r4, fp
	beq	.L5
	b	.L6
.L5:
	mov	fp, #5
	add	r0, r0, fp
	mov	fp, #0
	cmp	r0, fp
	beq	.L6
	mov	r10, #114
	mul	r0, r10, r1
	mov	fp, #0
	cmp	r0, fp
	beq	.L1
.L6:
	mov	r10, #11
	mov	r0, r10
	mov	fp, #10
	cmp	r5, fp
	ble	.L7
	b	.L1
.L7:
	b	.L0
.L1:
	add	sp, sp, #0
	pop	{r4-r6, r10, fp, pc}
.text
.global	func3
.type	func3, %function
.syntax unified
.thumb
.thumb_func
func3:
	push	{r4-r6, r10, fp, lr}
	sub	sp, sp, #40
	mov	r5, r0
	mov	r0, r1
	mov	r0, r2
	mov	r0, r3
	mov	fp, #5
	add	r0, r5, fp
	add	fp, sp, #0
	mov	r10, #0
	add	fp, fp, r10, lsl #2
	str	r0, [fp]
	mov	fp, #4
	add	r0, r5, fp
	add	fp, sp, #0
	mov	r10, #1
	add	fp, fp, r10, lsl #2
	str	r0, [fp]
	add	fp, sp, #0
	mov	r10, #2
	add	fp, fp, r10, lsl #2
	mov	r10, #6
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #3
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #4
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #5
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #6
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #7
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #8
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #9
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	mvn	fp, #5
	cmp	r5, fp
	ble	.L12
	b	.L11
.L12:
	add	fp, sp, #0
	mov	r10, #3
	ldr	r0, [fp, r10, lsl #2]
	mov	fp, #9
	cmp	r0, fp
	ble	.L9
.L11:
	b	.L9
.L9:
	mov	r0, r6
	add	sp, sp, #40
	pop	{r4-r6, r10, fp, pc}
.text
.global	func1
.type	func1, %function
.syntax unified
.thumb
.thumb_func
func1:
	push	{r4-r5, r10, fp, lr}
	sub	sp, sp, #0
	mov	r4, r0
	mov	r0, r1
	mov	r0, r2
	mov	r0, r3
	mov	r0, r4
	mov	r10, #10
	mov	r1, r10
	bl	func2
	mov	r0, #10
	mov	r0, r0
	add	sp, sp, #0
	pop	{r4-r5, r10, fp, pc}
