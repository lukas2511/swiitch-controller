/*
 * bitops.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2012 Daniel Thompson <daniel@redfelineninja.org.uk>
 * Copyright (C) 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <assert.h>
#include <stdint.h>

#include "librfn/bitops.h"

int bitcnt(uint32_t x)
{
	uint32_t n;

	n = (x >> 1) & 0x77777777;
	x = x - n;
	n = (n >> 1) & 0x77777777;
	x = x - n;
	n = (n >> 1) & 0x77777777;
	x = x - n;
	x = (x + (x >> 4)) & 0x0F0F0F0F;
	x = x * 0x01010101;

	return x >> 24;
}

int clz(uint32_t x)
{
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >>16);
	return bitcnt(~x);
}

int ctz(uint32_t x)
{
	return bitcnt(~x & (x - 1));
}

int ilog2(uint32_t x)
{
	assert(x);
	return 31 - clz(x);
}
