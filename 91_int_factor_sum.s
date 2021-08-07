.arch armv7
.macro mov32, reg, val
	movw \reg, #:lower16:\val
	movt \reg, #:upper16:\val
.endm
.data
.global N
N:
	.word	0
.data
.global newline
newline:
	.word	0
.text
.global	factor
.type	factor, %function
.syntax unified
.thumb
.thumb_func
factor:
	push	{r4-r7, r10, fp, lr}
	sub	sp, sp, #0
	mov	r6, r0
	mov	r0, r1
	mov	r0, r2
	mov	r0, r3
	mov	r10, #0
	mov	r5, r10
	mov	r10, #1
	mov	r4, r10
.L0:
	mov	fp, #1
	add	r0, r6, fp
	cmp	r4, r0
	bge	.L1
	mov	r1, r4
	mov	r0, r6
	bl	__aeabi_idivmod
	mov	r0, r1
	mov	fp, #0
	cmp	r0, fp
	bne	.L2
	add	r5, r5, r4
.L2:
	mov	fp, #1
	add	r4, r4, fp
	b	.L0
.L1:
	mov	r0, r5
	add	sp, sp, #0
	pop	{r4-r7, r10, fp, pc}
.text
.global	main
.type	main, %function
.syntax unified
.thumb
.thumb_func
main:
	push	{r4-r5, r10, fp, lr}
	sub	sp, sp, #0
	mov	r5, r0
	mov	r0, r1
	mov	r0, r2
	mov	r0, r3
	mov	r10, #4
	mov	r10, r10
	mov32	fp, N
	str	r10, [fp]
	mov	r10, #10
	mov	r10, r10
	mov32	fp, newline
	str	r10, [fp]
	mov32	r10, #1478
	mov	r0, r10
	mov	r0, r0
	bl	factor
	mov	r1, r0
	mov	r0, r1
	add	sp, sp, #0
	pop	{r4-r5, r10, fp, pc}
