/* strcpy/stpcpy - copy a string returning pointer to start/end.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* To build as stpcpy, define BUILD_STPCPY before compiling this file.

   To test the page crossing code path more thoroughly, compile with
   -DSTRCPY_TEST_PAGE_CROSS - this will force all unaligned copies through
   the slower entry path.  This option is not intended for production use.  */

#define L(x) _ ## x

#define LDNSTN	0

/* Assumptions:
 *
 * ARMv8-a, AArch64, unaligned accesses, min page size 4k.
 */

	.global iso_conv_s

dstin	.req	x0
src	.req	x1
len	.req	x2
res	.req	x3
fps	.req	x4

vt1	.req	v1
vt2	.req	v2
vt3	.req	v3
vt4	.req	v4

iso_conv_s:
	mov	res, len
	subs	len, len, #32
	b.lt	.Ll8
	msr	fpsr, xzr

.Ln32:
#if LDNSTN
	ld1	{v1.8h-v4.8h}, [src]
#else
	ldr 	q1, [src]
	ldr 	q2, [src, 16]
	ldr 	q3, [src, 32]
	ldr 	q4, [src, 48]
#endif
	uqxtn	v1.8b, v1.8h
	uqxtn2	v1.16b, v2.8h
	uqxtn	v2.8b, v3.8h
	uqxtn2	v2.16b, v4.8h

	mrs	x4, fpsr
	cbnz	w4, .Ll8
#if LDNSTN
	st1	{v1.16b-v2.16b}, [dstin],#32
#else
	str	q1, [dstin], #16
	str	q2, [dstin], #16
#endif
	subs	len, len, #32
	add	src, src, #64
	b.ge	.Ln32

.Ll8:
	adds	len, len, #24
	b.lt	.Ll1
	msr	fpsr, xzr

.Ln8:
	ld1	{v1.8h}, [src]
	uqxtn	v1.8b, v1.8h
	mrs	x4, fpsr
	cbnz	w4, .Ll1
	st1	{v1.8b}, [dstin], #8
	subs	len, len, #8
	add	src, src, #16
	b.ge	.Ln8

.Ll1:
	adds	len, len, #8
	b.le	.Ldone

.Ln1:
	ldrh	w4, [src], #2
	tst	w4, #0xff00
	b.ne	.Ldone
	strb	w4, [dstin], #1
	subs	len, len, #1
	b.gt	.Ln1

.Ldone:
	sub	dstin, res, len
	ret
