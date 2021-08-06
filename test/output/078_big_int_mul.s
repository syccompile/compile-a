.arch armv7
.macro mov32, reg, val
	movw \reg, #:lower16:\val
	movt \reg, #:upper16:\val
.endm
.data
.global len
len:
	.word	20
.text
.global	main
.type	main, %function
.syntax unified
.thumb
.thumb_func
main:
	push	{r4-r9, r10, fp, lr}
	sub	sp, sp, #28
	sub	sp, sp, #512
	mov	r4, r0
	mov	r0, r1
	mov	r0, r2
	mov	r0, r3
	add	fp, sp, #0
	mov	r10, #0
	add	fp, fp, r10, lsl #2
	mov	r10, #1
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #1
	add	fp, fp, r10, lsl #2
	mov	r10, #2
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #2
	add	fp, fp, r10, lsl #2
	mov	r10, #3
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #3
	add	fp, fp, r10, lsl #2
	mov	r10, #4
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #4
	add	fp, fp, r10, lsl #2
	mov	r10, #5
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #5
	add	fp, fp, r10, lsl #2
	mov	r10, #6
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #6
	add	fp, fp, r10, lsl #2
	mov	r10, #7
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #7
	add	fp, fp, r10, lsl #2
	mov	r10, #8
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #8
	add	fp, fp, r10, lsl #2
	mov	r10, #9
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #9
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #10
	add	fp, fp, r10, lsl #2
	mov	r10, #1
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #11
	add	fp, fp, r10, lsl #2
	mov	r10, #2
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #12
	add	fp, fp, r10, lsl #2
	mov	r10, #3
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #13
	add	fp, fp, r10, lsl #2
	mov	r10, #4
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #14
	add	fp, fp, r10, lsl #2
	mov	r10, #5
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #15
	add	fp, fp, r10, lsl #2
	mov	r10, #6
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #16
	add	fp, fp, r10, lsl #2
	mov	r10, #7
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #17
	add	fp, fp, r10, lsl #2
	mov	r10, #8
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #18
	add	fp, fp, r10, lsl #2
	mov	r10, #9
	str	r10, [fp]
	add	fp, sp, #0
	mov	r10, #19
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #0
	add	fp, fp, r10, lsl #2
	mov	r10, #2
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #1
	add	fp, fp, r10, lsl #2
	mov	r10, #3
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #2
	add	fp, fp, r10, lsl #2
	mov	r10, #4
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #3
	add	fp, fp, r10, lsl #2
	mov	r10, #2
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #4
	add	fp, fp, r10, lsl #2
	mov	r10, #5
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #5
	add	fp, fp, r10, lsl #2
	mov	r10, #7
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #6
	add	fp, fp, r10, lsl #2
	mov	r10, #9
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #7
	add	fp, fp, r10, lsl #2
	mov	r10, #9
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #8
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #9
	add	fp, fp, r10, lsl #2
	mov	r10, #1
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #10
	add	fp, fp, r10, lsl #2
	mov	r10, #9
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #11
	add	fp, fp, r10, lsl #2
	mov	r10, #8
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #12
	add	fp, fp, r10, lsl #2
	mov	r10, #7
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #13
	add	fp, fp, r10, lsl #2
	mov	r10, #6
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #14
	add	fp, fp, r10, lsl #2
	mov	r10, #4
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #15
	add	fp, fp, r10, lsl #2
	mov	r10, #3
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #16
	add	fp, fp, r10, lsl #2
	mov	r10, #2
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #17
	add	fp, fp, r10, lsl #2
	mov	r10, #1
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #18
	add	fp, fp, r10, lsl #2
	mov	r10, #2
	str	r10, [fp]
	add	fp, sp, #80
	mov	r10, #19
	add	fp, fp, r10, lsl #2
	mov	r10, #2
	str	r10, [fp]
	mov	r10, #20
	mov	r10, r10
	add	fp, sp, #8
	add	fp, fp, #512
	str	r10, [fp]
	mov	r10, #20
	mov	r0, r10
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #0
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #1
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #2
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #3
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #4
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #5
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #6
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #7
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #8
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #9
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #10
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #11
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #12
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #13
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #14
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #15
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #16
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #17
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #18
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #19
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #20
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #21
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #22
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #23
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #24
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #25
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #26
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #27
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #28
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #29
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #30
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #31
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #32
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #33
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #34
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #35
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #36
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #37
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #38
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #39
	add	fp, fp, r10, lsl #2
	mov	r10, #0
	str	r10, [fp]
	mov	r10, #0
	mov	r4, r10
.L0:
	add	fp, sp, #8
	add	fp, fp, #512
	ldr	fp, [fp]
	cmp	r4, fp
	bge	.L1
	mov	r10, #0
	mov	r5, r10
	mov	r7, r4
	add	r5, r5, r7
	add	fp, sp, #0
	ldr	r5, [fp, r5, lsl #2]
	mov	r10, #0
	mov	r7, r10
	mov	r9, r4
	add	r7, r7, r9
	add	fp, sp, #160
	add	fp, fp, r7, lsl #2
	str	r5, [fp]
	mov	fp, #1
	add	r4, r4, fp
	b	.L0
.L1:
	mov	r10, #0
	mov	r4, r10
.L2:
	cmp	r4, r0
	bge	.L3
	mov	r10, #0
	mov	r5, r10
	mov	r7, r4
	add	r5, r5, r7
	add	fp, sp, #80
	ldr	r5, [fp, r5, lsl #2]
	mov	r10, #0
	mov	r7, r10
	mov	r9, r4
	add	r7, r7, r9
	add	fp, sp, #4
	add	fp, fp, #256
	add	fp, fp, r7, lsl #2
	str	r5, [fp]
	mov	fp, #1
	add	r4, r4, fp
	b	.L2
.L3:
	add	r10, sp, #8
	add	r10, r10, #512
	ldr	r10, [r10]
	add	fp, r10, r0
	add	r10, sp, #12
	add	r10, r10, #512
	str	fp, [r10]
	add	r10, sp, #12
	add	r10, r10, #512
	ldr	r10, [r10]
	mov	r4, r10
	mov	fp, #1
	sub	r9, r4, fp
	mov	r10, #0
	mov	r4, r10
.L4:
	cmp	r4, r9
	bgt	.L5
	mov	r10, #0
	mov	r5, r10
	mov	r7, r4
	add	r5, r5, r7
	add	fp, sp, #104
	add	fp, fp, #256
	add	fp, fp, r5, lsl #2
	mov	r10, #0
	str	r10, [fp]
	mov	fp, #1
	add	r4, r4, fp
	b	.L4
.L5:
	mov	r10, #0
	mov	r6, r10
	mov	fp, #1
	sub	r4, r0, fp
.L6:
	mvn	fp, #0
	cmp	r4, fp
	ble	.L7
	mov	r10, #0
	mov	r0, r10
	mov	r5, r4
	add	r0, r0, r5
	add	fp, sp, #4
	add	fp, fp, #256
	ldr	r0, [fp, r0, lsl #2]
	mov	r7, r0
	add	r10, sp, #8
	add	r10, r10, #512
	ldr	r10, [r10]
	mov	fp, #1
	sub	r5, r10, fp
.L8:
	mvn	fp, #0
	cmp	r5, fp
	ble	.L9
	mov	r10, #0
	mov	r0, r10
	mov	r6, r9
	add	r0, r0, r6
	add	fp, sp, #104
	add	fp, fp, #256
	ldr	r0, [fp, r0, lsl #2]
	mov	r10, #0
	mov	r6, r10
	mov	r10, r5
	add	fp, sp, #16
	add	fp, fp, #512
	str	r10, [fp]
	add	fp, sp, #16
	add	fp, fp, #512
	ldr	fp, [fp]
	add	r6, r6, fp
	add	fp, sp, #160
	ldr	r6, [fp, r6, lsl #2]
	mul	r6, r7, r6
	add	r6, r0, r6
	mov	fp, #10
	cmp	r6, fp
	blt	.L10
	mov	r10, #0
	mov	r0, r10
	mov	r1, r9
	add	r0, r0, r1
	add	fp, sp, #104
	add	fp, fp, #256
	add	fp, fp, r0, lsl #2
	str	r6, [fp]
	mov	fp, #1
	sub	fp, r9, fp
	add	r10, sp, #20
	add	r10, r10, #512
	str	fp, [r10]
	add	r10, sp, #20
	add	r10, r10, #512
	ldr	r10, [r10]
	mov	r0, r10
	mov	r10, #0
	mov	r10, r10
	add	fp, sp, #16
	add	fp, fp, #512
	str	r10, [fp]
	mov	r1, r0
	add	r10, sp, #16
	add	r10, r10, #512
	ldr	r10, [r10]
	add	fp, r10, r1
	add	r10, sp, #16
	add	r10, r10, #512
	str	fp, [r10]
	add	fp, sp, #104
	add	fp, fp, #256
	add	r10, sp, #16
	add	r10, r10, #512
	ldr	r10, [r10]
	ldr	r10, [fp, r10, lsl #2]
	add	fp, sp, #16
	add	fp, fp, #512
	str	r10, [fp]
	mov	r10, #10
	mov	r1, r10
	mov	r0, r6
	bl	__aeabi_idiv
	mov	r8, r0
	add	r10, sp, #16
	add	r10, r10, #512
	ldr	r10, [r10]
	add	r0, r10, r8
	add	r10, sp, #20
	add	r10, r10, #512
	ldr	r10, [r10]
	mov	r10, r10
	add	fp, sp, #16
	add	fp, fp, #512
	str	r10, [fp]
	mov	r10, #0
	mov	r8, r10
	add	r10, sp, #16
	add	r10, r10, #512
	ldr	r10, [r10]
	mov	r10, r10
	add	fp, sp, #24
	add	fp, fp, #512
	str	r10, [fp]
	add	fp, sp, #24
	add	fp, fp, #512
	ldr	fp, [fp]
	add	r8, r8, fp
	add	fp, sp, #104
	add	fp, fp, #256
	add	fp, fp, r8, lsl #2
	str	r0, [fp]
	b	.L11
.L10:
	mov	r10, #0
	mov	r0, r10
	mov	r10, r9
	add	fp, sp, #16
	add	fp, fp, #512
	str	r10, [fp]
	add	fp, sp, #16
	add	fp, fp, #512
	ldr	fp, [fp]
	add	r0, r0, fp
	add	fp, sp, #104
	add	fp, fp, #256
	add	fp, fp, r0, lsl #2
	str	r6, [fp]
.L11:
	mov	fp, #1
	sub	r5, r5, fp
	mov	fp, #1
	sub	r9, r9, fp
	b	.L8
.L9:
	add	fp, sp, #8
	add	fp, fp, #512
	ldr	fp, [fp]
	add	r0, r9, fp
	mov	fp, #1
	sub	r9, r0, fp
	mov	fp, #1
	sub	r4, r4, fp
	b	.L6
.L7:
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #0
	ldr	r0, [fp, r10, lsl #2]
	mov	fp, #0
	cmp	r0, fp
	beq	.L12
	add	fp, sp, #104
	add	fp, fp, #256
	mov	r10, #0
	ldr	r0, [fp, r10, lsl #2]
	mov	r0, r0
	bl	putint
	mov	r4, r0
.L12:
	mov	r10, #1
	mov	r4, r10
.L14:
	add	r10, sp, #12
	add	r10, r10, #512
	ldr	r10, [r10]
	mov	r0, r10
	mov	fp, #1
	sub	r0, r0, fp
	cmp	r4, r0
	bgt	.L15
	mov	r10, #0
	mov	r0, r10
	mov	r5, r4
	add	r0, r0, r5
	add	fp, sp, #104
	add	fp, fp, #256
	ldr	r0, [fp, r0, lsl #2]
	mov	r0, r0
	bl	putint
	mov	r5, r0
	mov	fp, #1
	add	r4, r4, fp
	b	.L14
.L15:
	mov	r0, #0
	mov	r0, r0
	add	sp, sp, #28
	add	sp, sp, #512
	pop	{r4-r9, r10, fp, pc}
