/*
 * benchmark.c
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

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "librfn.h"

void rf_benchmark_init(rf_benchmark_t *b, uint64_t runtime)
{
	b->end = 0;
	b->start = time64_now() + 2;
	b->expiry = b->start + runtime;

	// wait until the test starts
	while (b->start > time64_now())
		; // do nothing
}

bool rf_benchmark_running(rf_benchmark_t *b)
{
	b->end = time64_now();
	return (b->end < b->expiry);
}

void rf_benchmark_finalize(rf_benchmark_t *b, uint64_t nominal, rf_benchmark_results_t *r)
{
	uint64_t now = time64_now();
	if (b->end < b->expiry)
		b->end = now;

	uint64_t elapsed = b->end - b->start;

	r->ratio = (double) nominal / (double) elapsed;
	r->cpu_usage = 100.0 / r->ratio;
}

void rf_benchmark_results_show(rf_benchmark_results_t *r, const char *tag)
{
	char *prefix = getenv("BENCHMARK_PREFIX");

	printf("%s%-40s  Bandwidth %6.2fx    CPU load %6.3f%%\n",
			(prefix ? prefix : ""),
			tag, r->ratio, r->cpu_usage);
}
