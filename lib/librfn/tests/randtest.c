/*
 * randtest.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2012 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#undef NDEBUG

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <librfn.h>

int main()
{
	uint32_t seed = 1;
	uint32_t r;
	for (int i=1; i<10000; i++) {
		r = rand31_r(&seed);
		if (i<=10) {
			printf("%10d%8d\n", r, i);
		}
		(void) rand31_r(&r);
	}
	assert(r == 1043618065);

	return 0;
}
