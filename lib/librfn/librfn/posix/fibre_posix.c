/*
 * fibre_posix.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2013 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "librfn/fibre.h"
#include "librfn/time.h"
#include "librfn/util.h"

void fibre_scheduler_main_loop()
{
	while (true) {
		uint32_t sleep_until = fibre_scheduler_next(time_now());
		int32_t sleep_interval = cyclecmp32(sleep_until, time_now());
		sleep_interval = sleep_interval < 1000 ? sleep_interval : 50000;
		if (sleep_interval > 0)
			usleep(sleep_interval);
	}
}
