/*
 * libbench.c
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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "librfn.h"
#include "libbench.h"

/*
 * Assuming time_now() ticks at 1MHz (microseconds) then a million cycles
 * results in a results in picoseconds per cycle. This is great for
 * benchmarking powerful multi-GHz desktop machines. If BENCHMARK_SLOW_MACHINE
 * is set we reduce the number of cycles and the results will be in
 * nanoseconds per cycle.
 */
#ifdef BENCHMARK_SLOW_MACHINE
#define NUM_CYCLES 1000
#else
#define NUM_CYCLES 1000000
#endif

static fibre_t *next_action;

typedef struct {
	uint32_t start_time;
	uint32_t end_time;
	unsigned int cycles;
	unsigned int count;
	fibre_t fibre;
	fibre_t *friend;
} benchmark_fibre_t;

static int yield_fibre(fibre_t *fibre)
{
	benchmark_fibre_t *bm = containerof(fibre, benchmark_fibre_t, fibre);

	PT_BEGIN_FIBRE(fibre);

	bm->start_time = time_now();
	bm->count = 0;

	while (bm->count++ < bm->cycles)
		PT_YIELD();

	bm->end_time = time_now();
	fibre_run(next_action);
	PT_END();
}

static int run_fibre(fibre_t *fibre)
{
	benchmark_fibre_t *bm = containerof(fibre, benchmark_fibre_t, fibre);

	PT_BEGIN_FIBRE(fibre);

	bm->start_time = time_now();
	bm->count = 0;

	while (bm->count++ < bm->cycles) {
		fibre_run(bm->friend);
		PT_WAIT();
	}

	/* if we are fibre[0] we need to poke fibre[1] one last time */
	if (bm->friend > fibre)
		fibre_run(bm->friend);

	bm->end_time = time_now();
	fibre_run(next_action);
	PT_END();
}

static int atomic_run_fibre(fibre_t *fibre)
{
	benchmark_fibre_t *bm = containerof(fibre, benchmark_fibre_t, fibre);

	PT_BEGIN_FIBRE(fibre);

	bm->start_time = time_now();
	bm->count = 0;

	while (bm->count++ < bm->cycles) {
		fibre_run_atomic(bm->friend);
		PT_WAIT();
	}

	/* if we are fibre[0] we need to poke fibre[1] one last time */
	if (bm->friend > fibre)
		fibre_run_atomic(bm->friend);

	bm->end_time = time_now();
	fibre_run(next_action);
	PT_END();
}

static benchmark_fibre_t single_yield = {
	.cycles = NUM_CYCLES,
	.fibre = FIBRE_VAR_INIT(yield_fibre)
};

static benchmark_fibre_t paired_yield[2] = {
	{
		.cycles = NUM_CYCLES/2,
		.fibre = FIBRE_VAR_INIT(yield_fibre)
	},
	{
		.cycles = NUM_CYCLES/2,
		.fibre = FIBRE_VAR_INIT(yield_fibre)
	}
};

static benchmark_fibre_t simple_run[2] = {
	{
		.cycles = NUM_CYCLES/2,
		.fibre = FIBRE_VAR_INIT(run_fibre),
		.friend = &simple_run[1].fibre,
	},
	{
		.cycles = NUM_CYCLES/2,
		.fibre = FIBRE_VAR_INIT(run_fibre),
		.friend = &simple_run[0].fibre,
	},
};

static benchmark_fibre_t atomic_run[2] = {
	{
		.cycles = NUM_CYCLES/2,
		.fibre = FIBRE_VAR_INIT(atomic_run_fibre),
		.friend = &atomic_run[1].fibre,
	},
	{
		.cycles = NUM_CYCLES/2,
		.fibre = FIBRE_VAR_INIT(atomic_run_fibre),
		.friend = &atomic_run[0].fibre,
	},
};


void benchmark_init(benchmark_results_t *results, fibre_t *wakeup)
{
	memset(results, 0, sizeof(*results));
	for (int i=0; i<lengthof(results->stats); i++)
		stats_init(&results->stats[i]);
	results->wakeup = wakeup;
}	

int benchmark_run_once(benchmark_results_t *results)
{
	/* next_action is a bit of a hack but since it is meaningless to run
	 * two benchmarks concurrently it is sufficient to unconditionally
	 * update this every time we run.
	 */
	next_action = results->wakeup;

	PT_BEGIN(&results->pt);

	fibre_run(&single_yield.fibre);
	PT_WAIT();
	stats_add(&results->stats[BENCHMARK_SINGLE],
		  single_yield.end_time - single_yield.start_time);

	fibre_run(&paired_yield[0].fibre);
	fibre_run(&paired_yield[1].fibre);
	PT_WAIT();
	stats_add(&results->stats[BENCHMARK_PAIRED],
		  paired_yield[1].end_time - paired_yield[0].start_time);

	fibre_run(&simple_run[0].fibre);
	PT_WAIT();
	stats_add(&results->stats[BENCHMARK_SIMPLE_RUN],
		  simple_run[1].end_time - simple_run[0].start_time);

	fibre_run(&atomic_run[0].fibre);
	PT_WAIT();
	stats_add(&results->stats[BENCHMARK_ATOMIC_RUN],
		  atomic_run[1].end_time - atomic_run[0].start_time);

	PT_END();
}

static const char *lookup_name(int n)
{
	static char lower[16];
#define C(x) case BENCHMARK_ ## x: strncpy(lower, #x, sizeof(lower)); break
	switch (n) {
	C(SINGLE);
	C(PAIRED);
	C(SIMPLE_RUN);
	C(ATOMIC_RUN);
	default:
		return NULL;
#undef C
	}

	lower[sizeof(lower)-1] = '\0';
	(void) strtolower(lower);

	return lower;
}

const char *benchmark_get_result(benchmark_results_t *results, int n,
				 stats_t *s)
{
	const char *retval = lookup_name(n);
	if (retval)
		memcpy(s, &results->stats[n], sizeof(*s));

	return retval;
}

void benchmark_show_results(benchmark_results_t *results)
{
	const char *name;
	stats_t result;

	printf("Test                   Min      Mean       Max\n");
	printf("----------------------------------------------\n");

	for (int i = 0; (name = benchmark_get_result(results, i, &result)); i++)
		printf("%-16s%11d%11d%11d\n", name,
		       result.min, stats_mean(&result), result.max);
}	

void benchmark_show_csv(benchmark_results_t *results, FILE *f)
{
	const char *name;
	stats_t result;

	fprintf(f,"\"Test\",\"Min\",\"Mean\",\"Max\"\n");
	for (int i = 0; (name = benchmark_get_result(results, i, &result)); i++)
		fprintf(f, "\"%s\",%d,%d,%d\n", name,
		        result.min, stats_mean(&result), result.max);
}
