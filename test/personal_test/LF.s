.arch armv7
.macro mov32, reg, val
	movw \reg, #:lower16:\val
	movt \reg, #:upper16:\val
.endm
.data
.global n
n:
	.word	0
.text
.global	foo
.type	foo, %function
.syntax unified
.thumb
.thumb_func
foo:
	push	{r4-r5, r10, fp, lr}
	sub	sp, sp, #0
	mov	r4, r0
	mov	r0, r1
	mov	r0, r2
	mov	r0, r3
	mov32	r10, n
	ldr	r10, [r10]
	mov	r0, r10
	mov	r10, #5
	add	fp, r4, r10, lsl #2
	str	r0, [fp]
	add	sp, sp, #0
	pop	{r4-r5, r10, fp, pc}
.text
.global	main
.type	main, %function
.syntax unified
.thumb
.thumb_func
main:
	push	{r4-r5, r10, fp, lr}
	sub	sp, sp, #32
	mov	r5, r0
	mov	r0, r1
	mov	r0, r2
	mov	r0, r3
	add	fp, sp, #0
	mov	r10, #0
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #1
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #2
	add	fp, fp, r10, lsl #2
	mov	r10, #0
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
	mov	r0, #0
	mov	r0, r0
	add	sp, sp, #32
	pop	{r4-r5, r10, fp, pc}
