/*
 * libbench.h
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

/*!
 * \file 
 * \brief Micro benchmark suite for some librfn features
 *
 * libbench is example code that performs benchmarks on certain librfn features
 * including the fibre scheduler.
 *
 * Note that the benchmark suite measures elapsed time, rather than CPU load,
 * and for this reason does not use the benchmark module of librfn (because
 * the benchmark module is designed to estimate CPU load for DSP activities).
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "librfn.h"

enum {
	BENCHMARK_SINGLE,
	BENCHMARK_PAIRED,
	BENCHMARK_SIMPLE_RUN,
	BENCHMARK_ATOMIC_RUN,
	BENCHMARK_MAX
};

typedef struct {
	pt_t pt;
	fibre_t *wakeup;
	stats_t stats[BENCHMARK_MAX];
} benchmark_results_t;

void benchmark_init(benchmark_results_t *results, fibre_t *wakeup);
int benchmark_run_once(benchmark_results_t *results);
const char *benchmark_get_result(benchmark_results_t *results, int n,
				 stats_t *s);
void benchmark_show_results(benchmark_results_t *results);
void benchmark_show_csv(benchmark_results_t *results, FILE *f);

