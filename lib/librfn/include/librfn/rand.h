/*
 * rand.h
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

#ifndef RF_RAND_H_
#define RF_RAND_H_

#include <stdint.h>

/*!
 * \defgroup librfn_rand Random numbers
 *
 * \brief Pseudo random number generators.
 *
 * The algorithms selected here tend towards the quick 'n dirty (and very
 * fast on microcontrollers) rather than the high quality state-of-the-art
 * ones you might expect in a random number library.
 *
 * @{
 */

/*!
 * Initialize a seed value used by rand31_r().
 *
 * The only actualy requirement on the initial seed is that it must be
 * strictly greater than zero (and 32-bit). However for simplicity we
 * choose 1.
 */
#define RAND31_VAR_INIT 1

/*!
 * Generate a 31-bit random number.
 *
 * From http://www.firstpr.com.au/dsp/rand31/ :
 * Park-Miller "minimal standard" 31 bit
 * pseudo-random number generator, implemented
 * with David G. Carta's optimization: with
 * 32 bit math and without division.
 *
 * \param [inout] seedp Current state of the PRNG, initalized using
 *                      ::RAND31_VAR_INIT
 */
uint32_t rand31_r(uint32_t *seedp);

/*! @} */
#endif // RF_RAND_H_
