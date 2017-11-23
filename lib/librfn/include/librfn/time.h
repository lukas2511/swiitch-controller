/*
 * time.h
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2013-2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef RF_TIME_H_
#define RF_TIME_H_

#include <stdint.h>

/*!
 * \defgroup librfn_time Time management
 *
 * \brief Easy to use, high resolution timer functions.
 *
 * @{
 */

/*!
 * Initialize the time sub-system.
 */
void time_init(void);

/*!
 * Get the current wall time in microseconds and modulo 2^32.
 *
 * The output of this function will wrap after slightly more than one hour.
 * There this function is only particularly useful for measuring small
 * intervals on small micro-controller systems where 64-bit arithmetic might
 * be regarded as expensive (either in cycles or memory use).
 */
uint32_t time_now(void);

/*!
 * Get the current wall time in microseconds.
 *
 * On POSIX systems this is merely a wrapper around clock_gettime() but
 * is often less cumbersome to work with.
 */
uint64_t time64_now(void);

/*! @} */
#endif // RF_TIME_H_
