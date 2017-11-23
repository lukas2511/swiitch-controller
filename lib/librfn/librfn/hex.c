/*
 * hex.c
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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "librfn.h"

static inline char hexchar(char h)
{
	if (h < 10)
		return '0' + h;

	return 'a' - 10 + h;
}

int hex_dump_to_file(FILE *f, unsigned char *p, size_t sz)
{
	size_t oldsz = sz;

	while (sz > 0) {
		for (int i=0;
		     i<16 && sz > 0;
		     i++, sz--, p++)
			fprintf(f, "%c%c", hexchar(*p >> 4), hexchar(*p & 0xf));
		fprintf(f, "\n");
	}

	return oldsz;
}

int hex_dump(unsigned char *p, size_t sz)
{
	return hex_dump_to_file(stdout, p, sz);
}

static inline int nibble(char h)
{
	if (h <= '9')
		return h - '0';

	return (h & ~('a' - 'A')) - 'A' + 10;
}


int hex_get_byte(const char *s, const char **p)
{
    next_line:
	if (s) {
		char *q = strchr(s, ':');
		if (q)
			s = q+1;
	} else {
		s = *p;
		if (!s)
			return -1;
	}

	while (isspace((int) *s))
		if (*s++ == '\n')
			goto next_line;

	/* lazy evaluation ensures we don't read past end of string */
	if ('0' == s[0] && 'x' == s[1])
		s += 2;

	if (isxdigit((int) s[0]) && isxdigit((int) s[1])) {
		*p = s + 2;
		return 16 * nibble(s[0]) | nibble(s[1]);
	}

	/* jump to the next line (if there is one) */
	s = *p = strchr(s, '\n');
	if (s++)
		goto next_line;

	return -1;
}
