/*
 * statstest.c
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
//#define VERBOSE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <librfn.h>

int main()
{
	stats_t s;

	stats_init(&s);
	for (int i = 0; i < 10; i++)
		stats_add(&s, 1000);
	verify(1000 == stats_mean(&s));
	verify(100000 == stats_per_million(&s, 100000)); 

	for (int i = 0; i < 5; i++) {
		stats_add(&s,  100);
		stats_add(&s, 1900);
	}
	verify(1000 == stats_mean(&s));
	verify(200000 == stats_per_million(&s, 100000)); 

	stats_init(&s);
	for (int i = 0; i < 10; i++)
		stats_add(&s, 1000);
	verify(1000 == stats_mean(&s));
	verify(100000 == stats_per_million(&s, 100000)); 

	return 0;
}
