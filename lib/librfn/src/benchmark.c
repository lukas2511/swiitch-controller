/*
 * benchmark.c
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

typedef struct {
	fibre_t fibre;
	benchmark_results_t results;
} conductor_fibre_t;

int conductor_fibre(fibre_t *fibre)
{
	conductor_fibre_t *c = containerof(fibre, conductor_fibre_t, fibre);
	static uint64_t end_time;
	static int i;

	PT_BEGIN_FIBRE(&c->fibre);

	benchmark_init(&c->results, fibre);

	/* busy wait for two seconds to try and open up the cpufreq governor */
	printf("Preparing ...");
	fflush(stdout);
	end_time = time64_now() + 2000000;
	while (time64_now() < end_time)
		PT_YIELD();
	printf(" done\n");

	printf("Benchmarking ");
	fflush(stdout);
	for (i=0; i<50; i++) {
		printf(".");
		fflush(stdout);
		PT_SPAWN(&c->results.pt, benchmark_run_once(&c->results));
	}
	printf(" done\nBenchmark completed in %4.2f seconds\n\n",
	       (time64_now() - end_time) / 1000000.0);

	benchmark_show_results(&c->results);

	FILE *f = fopen("benchmark-results.csv", "w");
	benchmark_show_csv(&c->results, f);
	fclose(f);

	exit(0);
	PT_END();
}

conductor_fibre_t conductor = {
	.fibre = FIBRE_VAR_INIT(conductor_fibre)
};

int main(int argc, char *argv[])
{
	fibre_run(&conductor.fibre);
	fibre_scheduler_main_loop();

	return 0;
}
