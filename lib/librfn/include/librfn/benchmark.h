/*
 * benchmark.h
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2012-2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef RF_BENCHMARK_H_
#define RF_BENCHMARK_H_

#include <stdbool.h>
#include <stdint.h>

/*!
 * \defgroup librfn_benchmark Benchmark
 *
 * \brief Benchmark mechanism for real-time processing modules.
 *
 * This module contains library routines designed to estimate the CPU load
 * of real-time data processing.
 *
 * Rather than measure the CPU load directly the routine is run as fast
 * as possible and the nominal time (number of minutes of data processed) is
 * compared to the elapsed time.
 *
 * \code
 * rf_benchmark_t bm;
 * uint64_t nominal;
 * rf_benchmark_results_t results;
 *
 * rf_benchmark_init(&m, 2000000);
 * do {
 *     process_1ms_of_data();
 *     nominal += 1000;
 * } while (rf_benchmark_running(&bm);
 * rf_benchmark_finalize(&bm, nominal, &results);
 * rf_benchmark_results_show(&results, "process_1ms_of_data:");
 * \endcode
 * @{
 */

typedef struct {
	uint64_t start;
	uint64_t expiry;
	uint64_t end;
} rf_benchmark_t;

typedef struct {
	double ratio; //!< Degree by which we run faster than real time (x2 mean twice)
	double cpu_usage; //!< Estimated CPU usage at x1
} rf_benchmark_results_t;

/*!
 * Initialize a benchmark structure.
 *
 * \arg runtime Minimum time (in microseconds) that the benchmark must run for.
 */
void rf_benchmark_init(rf_benchmark_t *b, uint64_t runtime);

/*!
 * Test whether the benchmark has completed it's minimum runtime.
 */
bool rf_benchmark_running(rf_benchmark_t *b);

/*!
 * Generate the benchmark results by comparing the actual runtime to the nominal runtime.
 */
void rf_benchmark_finalize(rf_benchmark_t *b, uint64_t nominal, rf_benchmark_results_t *r);

void rf_benchmark_results_show(rf_benchmark_results_t *r, const char *tag);

/*! @} */
#endif // RF_BENCHMARK_H_
