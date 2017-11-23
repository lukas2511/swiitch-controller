/*
 * utiltest.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2012, 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
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

struct alpha {
	int a;
} a;

struct bravo {
	struct alpha a;
	int b;
} b;

struct charlie {
	struct alpha a;
	struct bravo b;
	int c;
} c;

int main()
{
	struct alpha *ap = &b.a;
	verify(&b == containerof(ap, struct bravo, a));

	struct bravo *bp = &c.b;
	verify(&c == containerof(bp, struct charlie, b));

	/* test ratelimit_check() (including resumption after limiting */
	static ratelimit_state_t rs;
	verify(ratelimit_check(&rs, 3, 10));
	verify(ratelimit_check(&rs, 3, 10));
	verify(ratelimit_check(&rs, 3, 10));
	verify(!ratelimit_check(&rs, 3, 10));
	verify(!ratelimit_check(&rs, 3, 10));
	rs.time -= 11*10000000; /* move time 11s forwards... */
	verify(ratelimit_check(&rs, 3, 10));
	verify(ratelimit_check(&rs, 3, 10));
	verify(ratelimit_check(&rs, 3, 10));
	verify(!ratelimit_check(&rs, 3, 10));

	/* check RATELIMIT() (no wait for resumption since that's too time consuming */
	int counter = 0;
	for (int i=0; i<10; i++) {
		RATELIMIT(counter++);
		if (i < 3)
			verify(counter == i+1);
	}
	verify(counter == 3);

	return 0;
}
