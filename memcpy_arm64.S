/*
 * Copyright (C) 2013 ARM Ltd.
 * Copyright (C) 2013 Linaro.
 *
 * This code is based on glibc cortex strings work originally authored by Linaro
 * and re-licensed under GPLv2 for the Linux kernel. The original code can
 * be found @
 *
 * http://bazaar.launchpad.net/~linaro-toolchain-dev/cortex-strings/trunk/
 * files/head:/src/aarch64/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

	.global memcpy_s

#define USE_PRFM 0

#define STP 0
#define STNP 0
#define STR 1
#define STN 0
#define STQ 0

#if USE_PRFM
#define PRFM(x) prfm pldl1strm,[x]
#define PRFM2(x,y) prfm pldl1strm,[x,y]
#else
#define PRFM(x)
#define PRFM2(x,y)
#endif

/*
 * Copy a buffer from src to dest (alignment handled by the hardware)
 *
 * Parameters:
 *	x0 - dest
 *	x1 - src
 *	x2 - n
 * Returns:
 *	x0 - dest
 */
dstin	.req	x0
src	.req	x1
count	.req	x2
tmp1	.req	x3
tmp1w	.req	w3
tmp2	.req	x4
tmp2w	.req	w4
tmp3	.req	x5
tmp3w	.req	w5
dst	.req	x6

A_l	.req	x7
A_h	.req	x8
B_l	.req	x9
B_h	.req	x10
C_l	.req	x11
C_h	.req	x12
D_l	.req	x13
D_h	.req	x14

V_0	.req	v0
V_1	.req	v1
V_2	.req	v2
V_3	.req	v3

memcpy_s:
	PRFM(src)
	mov	dst, dstin
	cmp	count, #16
	/*When memory length is less than 16, the accessed are not aligned.*/
	b.lo	.Ltiny15

	neg	tmp2, src
	ands	tmp2, tmp2, #15/* Bytes to reach alignment. */
	b.eq	.LSrcAligned
	sub	count, count, tmp2
	/*
	* Copy the leading memory data from src to dst in an increasing
	* address order.By this way,the risk of overwritting the source
	* memory data is eliminated when the distance between src and
	* dst is less than 16. The memory accesses here are alignment.
	*/
	tbz	tmp2, #0, 1f
	ldrb	tmp1w, [src], #1
	strb	tmp1w, [dst], #1
1:
	tbz	tmp2, #1, 2f
	ldrh	tmp1w, [src], #2
	strh	tmp1w, [dst], #2
2:
	tbz	tmp2, #2, 3f
	ldr	tmp1w, [src], #4
	str	tmp1w, [dst], #4
3:
	tbz	tmp2, #3, .LSrcAligned
	ldr	tmp1, [src],#8
	str	tmp1, [dst],#8

.LSrcAligned:
	cmp	count, #64
	b.ge	.Lcpy_over64
	/*
	* Deal with small copies quickly by dropping straight into the
	* exit block.
	*/
.Ltail63:
	/*
	* Copy up to 48 bytes of data. At this point we only need the
	* bottom 6 bits of count to be accurate.
	*/
	ands	tmp1, count, #0x30
	b.eq	.Ltiny15
	cmp	tmp1w, #0x20
	b.eq	1f
	b.lt	2f
	ldp	A_l, A_h, [src], #16
	stp	A_l, A_h, [dst], #16
1:
	ldp	A_l, A_h, [src], #16
	stp	A_l, A_h, [dst], #16
2:
	ldp	A_l, A_h, [src], #16
	stp	A_l, A_h, [dst], #16
.Ltiny15:
	/*
	* Prefer to break one ldp/stp into several load/store to access
	* memory in an increasing address order,rather than to load/store 16
	* bytes from (src-16) to (dst-16) and to backward the src to aligned
	* address,which way is used in original cortex memcpy. If keeping
	* the original memcpy process here, memmove need to satisfy the
	* precondition that src address is at least 16 bytes bigger than dst
	* address,otherwise some source data will be overwritten when memove
	* call memcpy directly. To make memmove simpler and decouple the
	* memcpy's dependency on memmove, withdrew the original process.
	*/
	tbz	count, #3, 1f
	ldr	tmp1, [src], #8
	str	tmp1, [dst], #8
1:
	tbz	count, #2, 2f
	ldr	tmp1w, [src], #4
	str	tmp1w, [dst], #4
2:
	tbz	count, #1, 3f
	ldrh	tmp1w, [src], #2
	strh	tmp1w, [dst], #2
3:
	tbz	count, #0, .Lexitfunc
	ldrb	tmp1w, [src]
	strb	tmp1w, [dst]

.Lexitfunc:
	ret

.Lcpy_over64:
	PRFM2(src,128)
	subs	count, count, #128
	b.ge	.Lcpy_body_large
	/*
	* Less than 128 bytes to copy, so handle 64 here and then jump
	* to the tail.
	*/
	ldp	A_l, A_h, [src],#16
	stp	A_l, A_h, [dst],#16
	ldp	B_l, B_h, [src],#16
	ldp	C_l, C_h, [src],#16
	stp	B_l, B_h, [dst],#16
	stp	C_l, C_h, [dst],#16
	ldp	D_l, D_h, [src],#16
	stp	D_l, D_h, [dst],#16

	tst	count, #0x3f
	b.ne	.Ltail63
	ret

	/*
	* Critical loop.  Start at a new cache line boundary.  Assuming
	*/
	.p2align	7
.Lcpy_body_large:
	mov 	D_l, src
	lsr	D_h, count, #7
	cmp 	D_h, #0
	b.lt	4f
	mov	D_h, #PRFM_LINES
4:
	PRFM2(src,64)
	PRFM2(src,128)
	add	D_l, D_l, #128
	sub	D_h, D_h, #1
	cbnz	D_h, 4b
	/* pre-get 64 bytes data. */
#if STP
	ldp	A_l, A_h, [src],#16
	ldp	B_l, B_h, [src],#16
	ldp	C_l, C_h, [src],#16
	ldp	D_l, D_h, [src],#16
#elif STNP
	ldnp	A_l, A_h, [src]
	ldnp	B_l, B_h, [src, #16]
	ldnp	C_l, C_h, [src, #32]
	ldnp	D_l, D_h, [src, #48]
	add	src, src, #64

#elif STN
	ld1	{ V0.16b, V1.16b, V2.16b, V3.16b }, [src], #64
#elif STQ
	ldp	q0, q1, [src], #32
	ldp	q2, q3, [src], #32
#endif
1:
	/*
	* interlace the load of next 64 bytes data block with store of the last
	* loaded 64 bytes data.
	*/
	tbz	src, #6, 2f
	PRFM2(src,PRFM_STRIDE-64)
	PRFM2(src,PRFM_STRIDE)
2:


#if STP
	stp	A_l, A_h, [dst],#16
	ldp	A_l, A_h, [src],#16
	stp	B_l, B_h, [dst],#16
	ldp	B_l, B_h, [src],#16
	stp	C_l, C_h, [dst],#16
	ldp	C_l, C_h, [src],#16
	stp	D_l, D_h, [dst],#16
	ldp	D_l, D_h, [src],#16
#elif STNP
	stnp	A_l, A_h, [dst]
	ldnp	A_l, A_h, [src]
	stnp	B_l, B_h, [dst, #16]
	ldnp	B_l, B_h, [src, #16]
	stnp	C_l, C_h, [dst, #32]
	ldnp	C_l, C_h, [src, #32]
	stnp	D_l, D_h, [dst, #48]
	ldnp	D_l, D_h, [src, #48]

	add	dst, dst, #64
	add	src, src, #64
#elif STN
	st1	{ V0.16b, V1.16b, V2.16b, V3.16b }, [dst], #64
	ld1	{ V0.16b, V1.16b, V2.16b, V3.16b }, [src], #64
#elif STQ
	stp	q0, q1, [dst], #32
	ldp	q0, q1, [src], #32
	stp	q2, q3, [dst], #32
	ldp	q2, q3, [src], #32
#elif STR
	str	A_l, [dst], #8
	str	A_h, [dst], #8
	ldp	A_l, A_h, [src],#16
	str	B_l, [dst], #8
	str	B_h, [dst], #8
	ldp	B_l, B_h, [src],#16
	str	C_l, [dst], #8
	str	C_h, [dst], #8
	ldp	C_l, C_h, [src],#16
	str	D_l, [dst], #8
	str	D_h, [dst], #8
	ldp	D_l, D_h, [src],#16
#endif
	subs	count, count, #64
	b.ge	1b

#if STP
	stp	A_l, A_h, [dst],#16
	stp	B_l, B_h, [dst],#16
	stp	C_l, C_h, [dst],#16
	stp	D_l, D_h, [dst],#16
#elif STNP
	stnp	A_l, A_h, [dst]
	stnp	B_l, B_h, [dst, #16]
	stnp	C_l, C_h, [dst, #32]
	stnp	D_l, D_h, [dst, #48]

	add	dst, dst, #64
#elif STN
	st1	{ V0.16b, V1.16b, V2.16b, V3.16b }, [dst], #64
#elif STQ
	stp	q0, q1, [dst], #32
	stp	q2, q3, [dst], #32
#endif

	tst	count, #0x3f
	b.ne	.Ltail63
	ret
