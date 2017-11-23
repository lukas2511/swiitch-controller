/*
 * regdumpdemo.c
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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "librfn.h"

static const regdump_desc_t myreg[] = { { "MYREG" },
					{ "AF", 0x000f },
					{ "EN", 0x0010 },
					{ "OVF", 0x0020 },
					{ "RXI", 0x0040 },
					{ "TXI", 0x0080 },
					{ "CH", 0xff00 },
					{ NULL } };

int main(int argc, char *argv[])
{
	regdump(0xffff, myreg);
	regdump(0x0000, myreg);
	regdump(0x2041, myreg);

	return 0;
}
