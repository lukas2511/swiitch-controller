/*
 * console_libopencm3.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * This file was derived from libopencm3's usart_irq_printf.c example.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>,
 * Copyright (C) 2011 Piotr Esden-Tempski <piotr@esden.net>
 * Copyright (C) 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <librfn/console.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static pthread_t injector_thread;

static void *injector(void *p)
{
	console_t *c = p;
	int d;

	while ((d = getchar()) != -1)
		console_putchar(c, d);

	exit(0);
	return NULL;
}

static pt_state_t console_exit(console_t *c)
{
	exit(0);
}
static const console_cmd_t cmd_exit =
    CONSOLE_CMD_VAR_INIT("exit", console_exit);

void console_hwinit(console_t *c)
{
	int res = pthread_create(&injector_thread, NULL, injector, c);
	if (0 != res) {
		perror("Cannot start injector");
	}

	console_register(&cmd_exit);
}
