/*
 * util.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2012, 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <stdio.h>

#include "librfn.h"

int32_t cyclecmp32(uint32_t a, uint32_t b)
{
	return (int32_t) (a - b);
}

bool ratelimit_check(ratelimit_state_t *rs, uint32_t n, uint32_t window)
{
	int32_t delta = (int32_t) (rs->time - time_now());
	if (delta < 0 || delta > (signed) (window * 1000000)) {
		rs->time = time_now() + (window * 1000000);
		rs->count = 1;
		return true;
	}

	return (rs->count++) < n;
}

/*! Out of memory hook.
 *
 * \todo Currently out_of_memory cannot actually be hooked
 */
void rf_internal_out_of_memory(void)
{
	abort();
}

void *xmalloc(size_t sz)
{
	void *p = malloc(sz);
	if (!p)
		rf_internal_out_of_memory();
	return p;
}
