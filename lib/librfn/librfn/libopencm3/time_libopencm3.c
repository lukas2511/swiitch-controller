/*
 * time_libopencm3.c
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

#include "librfn/time.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#ifndef F_CPU
#error F_CPU is not defined
#endif


static uint64_t sys_tick_counter;

void time_init()
{
	/* Set the systick to interrupt once each millisecond */
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	systick_set_reload((F_CPU / (8 * 1000))-1);

	/* Start counting. */
	systick_interrupt_enable();
	systick_counter_enable();
}

void sys_tick_handler(void)
{
	sys_tick_counter += 1000;
}

uint32_t time_now()
{
	return sys_tick_counter;
}


uint64_t time64_now()
{
	return sys_tick_counter;
}
