.arch armv7
.macro mov32, reg, val
	movw \reg, #:lower16:\val
	movt \reg, #:upper16:\val
.endm
.data
.global N
N:
	.word	1024
.data
.global A
A:
	.space	4194304
.data
.global B
B:
	.space	4194304
.data
.global C
C:
	.space	4194304
.text
.global	mm
.type	mm, %function
.syntax unified
.thumb
.thumb_func
mm:
	push	{r4-r9, r10, fp, lr}
	sub	sp, sp, #12
	mov	r7, r0
	mov	r10, r1
	str	r10, [sp, #0]
	mov	r10, r2
	str	r10, [sp, #4]
	mov	r9, r3
	mov	r10, #0
	mov	r0, r10
	mov	r10, #0
	mov	r1, r10
	mov	r10, #0
	mov	r1, r10
.L0:
	cmp	r0, r7
	bge	.L1
	mov32	fp, #1024
	mul	r3, r0, fp
.L2:
	cmp	r1, r7
	bge	.L3
	add	r2, r3, r1
	add	fp, r9, r2, lsl #2
	mov	r10, #0
	str	r10, [fp]
	mov	fp, #1
	add	r1, r1, fp
	b	.L2
.L3:
	mov	fp, #1
	add	r0, r0, fp
	b	.L0
.L1:
	mov	r10, #0
	mov	r2, r10
	mov	r10, #0
	mov	r0, r10
	mov	r10, #0
	mov	r1, r10
.L4:
	cmp	r2, r7
	bge	.L5
.L6:
	cmp	r0, r7
	bge	.L7
	mov	r10, #0
	mov	r3, r10
	mov32	fp, #1024
	mul	r5, r0, fp
	mov	r3, r5
	add	r3, r5, r2
	ldr	fp, [sp, #0]
	ldr	r3, [fp, r3, lsl #2]
	mov	fp, #0
	cmp	r3, fp
	bne	.L8
	mov	fp, #1
	add	r0, r0, fp
	b	.L6
.L8:
.L10:
	cmp	r1, r7
	bge	.L11
	mov	r10, #0
	mov	r3, r10
	mov	r3, r5
	add	r3, r5, r1
	ldr	r8, [r9, r3, lsl #2]
	mov	r10, #0
	mov	r3, r10
	mov	r3, r5
	add	r3, r5, r2
	ldr	fp, [sp, #0]
	ldr	r3, [fp, r3, lsl #2]
	mov	r10, #0
	mov	r6, r10
	mov32	fp, #1024
	mul	fp, r2, fp
	str	fp, [sp, #8]
	ldr	r10, [sp, #8]
	mov	r6, r10
	ldr	r10, [sp, #8]
	add	r6, r10, r1
	ldr	fp, [sp, #4]
	ldr	r6, [fp, r6, lsl #2]
	mul	r3, r3, r6
	add	r8, r8, r3
	mov	r10, #0
	mov	r3, r10
	mov32	fp, #1024
	mul	r5, r0, fp
	mov	r3, r5
	add	r3, r5, r1
	add	fp, r9, r3, lsl #2
	str	r8, [fp]
	mov	fp, #1
	add	r1, r1, fp
	b	.L10
.L11:
	mov	fp, #1
	add	r0, r0, fp
	b	.L6
.L7:
	mov	fp, #1
	add	r2, r2, fp
	b	.L4
.L5:
	add	sp, sp, #12
	pop	{r4-r9, r10, fp, pc}
.text
.global	main
.type	main, %function
.syntax unified
.thumb
.thumb_func
main:
	push	{r4-r9, r10, fp, lr}
	sub	sp, sp, #4
	mov	r6, r0
	mov	r6, r1
	mov	r6, r2
	mov	r6, r3
	bl	getint
	mov	r6, r0
	mov	r10, #0
	mov	r7, r10
	mov	r10, #0
	mov	r8, r10
	mov	r10, #0
	mov	r8, r10
.L12:
	cmp	r7, r6
	bge	.L13
	mov32	fp, #1024
	mul	r9, r7, fp
.L14:
	cmp	r8, r6
	bge	.L15
	bl	getint
	add	fp, r9, r8
	str	fp, [sp, #0]
	mov32	r10, A
	mov	r5, r10
	ldr	r10, [sp, #0]
	add	fp, r5, r10, lsl #2
	str	r0, [fp]
	mov	fp, #1
	add	r8, r8, fp
	b	.L14
.L15:
	mov	fp, #1
	add	r7, r7, fp
	b	.L12
.L13:
	mov	r10, #0
	mov	r7, r10
	mov	r10, #0
	mov	r8, r10
.L16:
	cmp	r7, r6
	bge	.L17
	mov32	fp, #1024
	mul	r9, r7, fp
.L18:
	cmp	r8, r6
	bge	.L19
	bl	getint
	mov	r10, r9
	str	r10, [sp, #0]
	add	fp, r9, r8
	str	fp, [sp, #0]
	mov32	r10, B
	mov	r5, r10
	ldr	r10, [sp, #0]
	add	fp, r5, r10, lsl #2
	str	r0, [fp]
	mov	fp, #1
	add	r8, r8, fp
	b	.L18
.L19:
	mov	fp, #1
	add	r7, r7, fp
	b	.L16
.L17:
	bl	_sysy_starttime
	mov	r10, #0
	mov	r7, r10
.L20:
	mov	fp, #5
	cmp	r7, fp
	bge	.L21
	mov	r0, r6
	mov32	r10, A
	mov	r1, r10
	mov	r1, r1
	mov32	r10, B
	mov	r2, r10
	mov	r2, r2
	mov32	r10, C
	mov	r3, r10
	mov	r3, r3
	bl	mm
	mov	r0, r6
	mov32	r10, A
	mov	r1, r10
	mov	r1, r1
	mov32	r10, C
	mov	r2, r10
	mov	r2, r2
	mov32	r10, B
	mov	r3, r10
	mov	r3, r3
	bl	mm
	mov	fp, #1
	add	r7, r7, fp
	b	.L20
.L21:
	mov	r10, #0
	mov	r10, r10
	str	r10, [sp, #0]
	mov	r10, #0
	mov	r7, r10
.L22:
	cmp	r7, r6
	bge	.L23
	mov	r10, #0
	mov	r8, r10
.L24:
	cmp	r8, r6
	bge	.L25
	mov	r10, #0
	mov	r9, r10
	mov32	fp, #1024
	mul	r5, r7, fp
	mov	r9, r5
	add	r9, r5, r8
	mov32	r10, B
	mov	r5, r10
	ldr	r9, [r5, r9, lsl #2]
	ldr	r10, [sp, #0]
	add	fp, r10, r9
	str	fp, [sp, #0]
	mov	fp, #1
	add	r8, r8, fp
	b	.L24
.L25:
	mov	fp, #1
	add	r7, r7, fp
	b	.L22
.L23:
	bl	_sysy_stoptime
	ldr	r10, [sp, #0]
	mov	r0, r10
	bl	putint
	mov	r10, #10
	mov	r0, r10
	bl	putch
	mov	r0, #0
	mov	r0, r0
	add	sp, sp, #4
	pop	{r4-r9, r10, fp, pc}
