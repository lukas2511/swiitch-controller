/*
 * enumtest.c
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
#include <string.h>

#include <librfn.h>

void test()
{
	rf_enumtable_t lookup[] = RF_ENUMTABLE_INITIALIZER(
			{ "One One", 1 },
			{ "Two Two", 2 },
			{ "Three Three", 3 }
	);

	assert(RF_ENUM_OUT_OF_RANGE == rf_string2enum(lookup, "Zero Zero"));
	assert(1 == rf_string2enum(lookup, "One One"));
	assert(2 == rf_string2enum(lookup, "Two Two"));
	assert(3 == rf_string2enum(lookup, "Three Three"));

	assert(NULL == rf_enum2string(lookup, 0));
	assert(0 == strcmp("One One", rf_enum2string(lookup, 1)));
	assert(0 == strcmp("Two Two", rf_enum2string(lookup, 2)));
	assert(0 == strcmp("Three Three", rf_enum2string(lookup, 3)));
	assert(NULL == rf_enum2string(lookup, 4));
}

int main()
{
	test();
	return 0;
}
