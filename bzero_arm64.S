//
// Copyright (c) 2015  Cavium Inc. All rights reserved.
//

	// x0 = ptr; x1 =nbytes;
	// x2 = cache_line_size; x3 = x2-1
	.globl	bzero_s

bzero_s:
	mov	x3, #1
	mrs	x2, DCZID_EL0	// Read the "DC ZVA" ID Register
	and	x2, x2, #0xf	// Isolate 'BS' field
	add	x2, x2, #2	// Adjust bs to log2(num_bytes)
	lslv	x2, x3, x2	// 2^^bs
	sub	x3, x2, x3
1:
	cbz	x1, 9f
	tst	x0, x3
	beq	2f
	strb	wzr, [x0], #1
	sub	x1, x1, #1
	b	1b

2:
	cmp	x1, x2
	bls	3f
	dc	zva, x0
	add	x0, x0, x2
	sub	x1, x1, x2
	b	2b

3:
	cbz	x1, 9f
	strb	wzr, [x0], #1
	sub	x1, x1, #1
	b	3b

9:
	ret

