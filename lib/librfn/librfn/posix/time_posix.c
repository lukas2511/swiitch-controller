/*
 * time_posix.c
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

#include <stdint.h>
#include <time.h>

#include "librfn/time.h"

void time_init(void)
{
}

uint32_t time_now()
{
	struct timespec now;

	clock_gettime(CLOCK_REALTIME, &now);

	// this expression does overflow but will "promote" everything
	// to unsigned meaning the overflow behavior is well defined
	// (and right for this instance)
	return (now.tv_sec * (uint32_t) 1000000) + (now.tv_nsec / 1000);
}


uint64_t time64_now()
{
	struct timespec now;

	clock_gettime(CLOCK_REALTIME, &now);

	return (now.tv_sec * 1000000ull) + (now.tv_nsec / 1000);
}
