/*
 * enum.c
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

#include <string.h>

#include "librfn.h"

const char *rf_enum2string(rf_enumtable_t *t, int e)
{
	for (; t->s; t++) {
		if (t->e == e)
			return t->s;
	}

	return NULL;
}

int rf_string2enum(rf_enumtable_t *t, const char *s)
{
	for (; t->s; t++) {
		if (0 == strcmp(t->s, s))
			return t->e;
	}

	return RF_ENUM_OUT_OF_RANGE;
}
