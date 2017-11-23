/*
 * fuzz.h
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

#ifndef RF_FUZZ_H_
#define RF_FUZZ_H_

#include <assert.h>

/*!
 * \defgroup librfn_fuzz Fuzzy comparisions
 *
 * \brief Compare floating point numbers match within a tolerance.
 *
 * The tolerance can be specified in absolute terms or it can be relative
 * to the magnitude of the numbers being compared.
 *
 * @{
 */

/*! Test whether two values are equal to within a relative tolerance.
 *
 * \return True, if a and b are approximately equal.
 */
int fuzzcmp(double a, double b, double delta);

/*! Single precision equivalence test using a relative tolerance.
 *
 * See ::fuzzcmp
 */
int fuzzcmpf(float a, float b, float delta);

/*! Test whether two values are equal to within an absolute tolerance.
 */
int fuzzcmpe(double a, double b, double e);

/*! Single precision equivalence test using an absolute tolerance.
 *
 * See ::fuzzcmpe
 */
int fuzzcmpef(float a, float b, float e);

/*! Test whether two values are equal within the first N bits.
 */
int fuzzcmpb(double a, double b, int bits);

/*! Single precision equivalence test within the first N bits.
 */
int fuzzcmpbf(float a, float b, int bits);

/*! Assert two values are approximately equal.
 *
 * Currently this checks the values differ by no more than 10 parts
 * per million.
 */
#define asserteq(a, b) assert(fuzzcmp(a, b, 1.000010))

/*! @} */
#endif // RF_FUZZ_H_
