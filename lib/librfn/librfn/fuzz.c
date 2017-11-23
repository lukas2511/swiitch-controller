/*
 * fuzz.c
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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "librfn.h"

int fuzzcmp(double a, double b, double delta)
{
	double aa, ab;

	assert(delta >= 1.0);

	aa = fabs(a);
	ab = fabs(b);

	/* check for matching sign */
	if ((aa == a) != (ab == b))
		return 0;

	if (aa <= ab) {
		return (aa * delta) >= ab;
	} else {
		return (ab * delta) >= aa;
	}
}

int fuzzcmpf(float a, float b, float delta)
{
	float aa, ab;

	assert(delta >= 1.0f);

	aa = fabsf(a);
	ab = fabsf(b);

	/* check for matching sign */
	if ((aa == a) != (ab == b))
		return 0;

	if (aa <= ab) {
		return (aa * delta) >= ab;
	} else {
		return (ab * delta) >= aa;
	}
}

int fuzzcmpe(double a, double b, double e)
{
	return ((a <= b) && ((a+e)) >= b) ||
	       ((b <= a) && ((b+e)) >= a);
}

int fuzzcmpef(float a, float b, float e)
{
	return ((a <= b) && ((a+e)) >= b) ||
	       ((b <= a) && ((b+e)) >= a);
}

int fuzzcmpb(double a, double b, int bits)
{
	double delta = 1.0 + (1.0 / (double) (1 << bits));
	return fuzzcmp(a, b, delta);
}

int fuzzcmpbf(float a, float b, int bits)
{
	float delta = 1.0f + (1.0f / (float) (1 << bits));
	return fuzzcmpf(a, b, delta);
}
