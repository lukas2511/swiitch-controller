/*
 * regdump.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <librfn/regdump.h>

#include <stdio.h>
#include <librfn/bitops.h>

int fregdump_single(FILE *f, uintreg_t reg, const regdump_desc_t *desc,
		    int *state)
{
	int i = (*state)++;

	if (0 == i) {
		fprintf(f, "Register: %s\n", desc[0].name);
	} else {
		int shift = ctz(desc[i].mask);
		fprintf(f, "\t%-16s: 0x%" PRIxreg "\n", desc[i].name,
			(reg & desc[i].mask) >> shift);
	}

	if (!desc[*state].name)
		*state = 0;

	return *state;
}

void fregdump(FILE *f, uintreg_t reg, const regdump_desc_t *desc)
{
	int state = REGDUMP_STATE_VAR_INIT;
	while (fregdump_single(f, reg, desc, &state))
		;
}

int regdump_single(uintreg_t reg, const regdump_desc_t *desc, int *state)
{
	return fregdump_single(stdout, reg, desc, state);
}

void regdump(uintreg_t reg, const regdump_desc_t *desc)
{
	fregdump(stdout, reg, desc);
}
