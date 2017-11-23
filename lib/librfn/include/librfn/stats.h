/*
 * stats.h
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

#ifndef RF_STATS_H_
#define RF_STATS_H_

#include <stdint.h>
#include <string.h>

/*!
 * \defgroup librfn_stats Statistics
 *
 * \brief Simple statistics accumulation
 *
 * Statistics gathered are primarily min, mean and max values although
 * a few derived values such as event frequency and percentages of total
 * can also be generated.
 * @{
 */

#if defined STATS_USE_FLOAT
typedef float statval_t;
#elif defined STATS_USE_DOUBLE
typedef double statval_t;
#elif defined STATS_USE_UINT64
typedef uint64_t statval_t;
#else
typedef uint32_t statval_t;
#endif

typedef struct stats {
	statval_t min;
	statval_t max;
	statval_t accumulator;
	statval_t count;
} stats_t;
 
void stats_init(stats_t *s);
void stats_add(stats_t *s, statval_t d);

statval_t stats_mean(stats_t *s);
statval_t stats_per_million(stats_t *s, statval_t total);

/*! @} */
#endif // RF_STATS_H_
