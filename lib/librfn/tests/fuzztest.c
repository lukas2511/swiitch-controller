/*
 * siggentest.c
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

#undef NDEBUG

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <librfn.h>

void test()
{
	assert( fuzzcmp(10, 10, 1));
	assert(!fuzzcmp(10, 10.01, 1));
	assert( fuzzcmp(10, 10.01, 1.1));
	assert(!fuzzcmp(10, 10.01, 1.001));

	assert(!fuzzcmp(10, 9.99, 1));
	assert( fuzzcmp(10, 9.99, 1.1));
	assert(!fuzzcmp(10, 9.99, 1.001));

	assert( fuzzcmp(-10, -10, 1));
	assert(!fuzzcmp(-10, -10.01, 1));
	assert( fuzzcmp(-10, -10.01, 1.1));
	assert(!fuzzcmp(-10, -10.01, 1.001));
}

void testf()
{
	assert( fuzzcmpf(10, 10, 1));
	assert(!fuzzcmpf(10, 10.01, 1));
	assert( fuzzcmpf(10, 10.01, 1.2));
	//TODO: why this this not match the double precision behaviour
	//assert(!fuzzcmpf(10, 10.1, 1.01));

	assert(!fuzzcmpf(10, 9.99, 1));
	assert( fuzzcmpf(10, 9.99, 1.1));
	//assert(!fuzzcmpf(10, 9.99, 1.001));

	assert( fuzzcmpf(-10, -10, 1));
	assert(!fuzzcmpf(-10, -10.01, 1));
	assert( fuzzcmpf(-10, -10.01, 1.1));
	//assert(!fuzzcmpf(-10, -10.01, 1.001));
}

void teste()
{
	assert( fuzzcmpe(10, 10, 0));

	assert( fuzzcmpe(10, 10.01, 0.1));
	assert( fuzzcmpe(10, 10.01, 0.01));
	assert(!fuzzcmpe(10, 10.01, 0.001));

	assert( fuzzcmpe(10, 9.99, 0.1));
	assert( fuzzcmpe(10, 9.99, 0.01));
	assert(!fuzzcmpe(10, 9.99, 0.001));

	assert( fuzzcmpe(-10, -10, 0));

	assert( fuzzcmpe(-10, -10.01, 0.1));
	assert( fuzzcmpe(-10, -10.01, 0.01));
	assert(!fuzzcmpe(-10, -10.01, 0.001));

	assert( fuzzcmpe(-10, -9.99, 0.1));
	assert( fuzzcmpe(-10, -9.99, 0.01));
	assert(!fuzzcmpe(-10, -9.99, 0.001));

}

void testef()
{
	assert( fuzzcmpef(10, 10, 0));

	assert( fuzzcmpef(10, 10.01, 0.1));
	assert( fuzzcmpef(10, 10.5, 0.5));
	assert(!fuzzcmpef(10, 10.01, 0.001));

	assert( fuzzcmpef(10, 9.99, 0.1));
	assert( fuzzcmpef(10, 9.5, 0.5));
	assert(!fuzzcmpef(10, 9.99, 0.001));

	assert( fuzzcmpef(-10, -10, 0));

	assert( fuzzcmpef(-10, -10.01, 0.1));
	assert( fuzzcmpef(-10, -10.5, 0.5));
	assert(!fuzzcmpef(-10, -10.01, 0.001));

	assert( fuzzcmpef(-10, -9.99, 0.1));
	assert( fuzzcmpef(-10, -9.5, 0.5));
	assert(!fuzzcmpef(-10, -9.99, 0.001));
}

void testb()
{
	double a = 32767.0f;
	double b = 32766.0f;

	assert( fuzzcmpb(a, b, 4));
	assert(!fuzzcmpb(a, b, 20));
	assert( fuzzcmpb(a, b, 14));
	//assert( fuzzcmpb(a, b, 15)); // 15 is the exact expected boundary
	assert(!fuzzcmpb(a, b, 16));
}

void testbf()
{
	float a = 32767.0f;
	float b = 32766.0f;

	assert( fuzzcmpbf(a, b, 4));
	assert(!fuzzcmpbf(a, b, 20));
	assert( fuzzcmpbf(a, b, 14));
	//assert( fuzzcmpbf(a, b, 15)); // 15 is the exact expected boundary
	assert(!fuzzcmpbf(a, b, 16));
}

int main()
{
	test();
	testf();
	teste();
	testef();
	testb();
	testbf();
	return 0;
}
