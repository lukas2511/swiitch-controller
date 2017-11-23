/*
 * stats.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2015 Daniel Thompson <daniel@redfelineninja.org.uk> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "librfn/stats.h"

void stats_init(stats_t *s)
{
	memset(s, 0, sizeof(*s));
#ifdef STATS_USE_FLOAT
	s->min = FLT_MIN;
	s->max = FLT_MAX;
#elif defined STATS_USE_DOUBLE
	s->min = DBL_MIN;
	s->max = DBL_MAX;
#else
	s->min = (statval_t) -1ull;
#endif

}
void stats_add(stats_t *s, statval_t d)
{
	if (d < s->min)
		s->min = d;

	if (d > s->max)
		s->max = d;

	s->accumulator += d;
	s->count++;
}

statval_t stats_mean(stats_t *s)
{
#if defined STATS_USE_FLOAT || defined STATS_USE_DOUBLE
	return s->accumulator / s->count;
#else
	return (s->accumulator + s->count / 2)  / s->count;
#endif
}

statval_t stats_per_million(stats_t *s, statval_t total)
{
	/* having multiplied by a million we're prepared to overlook
	 * mathematical rounding when statval_t is an integer type...
	 */
	return 1000000ull * s->accumulator / total;
}

