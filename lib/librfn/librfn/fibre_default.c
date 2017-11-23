/*
 * fibre_default.c
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

#include "librfn/fibre.h"
#include "librfn/time.h"

void fibre_scheduler_main_loop()
{
	while (true) {
		(void) fibre_scheduler_next(time_now());
	}
}
