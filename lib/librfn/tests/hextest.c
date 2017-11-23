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
//#define VERBOSE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <librfn.h>

static unsigned long parse32(const char *s)
{
	unsigned long acc = 0;

	for (int i=0; i<4; i++) {
		int res = hex_get_byte(s, &s);
		if (res < 0)
			return acc;
		acc = (acc << 8) + res;
	}

	return acc;
}

void test_get_byte(void)
{
#define V(x) verify(x == parse32(#x))
	V(00);
	V(0x00);
	V(0x12345678);
	V(0x80000000);
	V(0xCCccCcCc);
	V(0xdeadbeef);
	V(0xfeedface);
	V(0xffffffff);
#undef V

	verify(0 == parse32("00000000"));
	verify(0x1234abcd == parse32("1234abcd"));

	/* single characters are not (correctly) parseable. hex_get_byte() is
	 * designed to decode a hexdump...  it does *not* use C lexing rules.
	 */
#define X(x) verify(x != parse32(#x))
	X(1);
	X(123);
#undef X
		
}

int main()
{
	test_get_byte();
	return 0;
}
