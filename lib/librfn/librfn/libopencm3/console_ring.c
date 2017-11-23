/*
 * console_ring.c
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

#include <stdio.h>
#include <errno.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include <librfn/console.h>

/* prototype functions not found in the headers (for -Wmissing-prototypes) */
int _write(int file, char *ptr, int len);

static uint8_t logbuf[1024];
static ringbuf_t logring = RINGBUF_VAR_INIT(logbuf, sizeof(logbuf));

static console_t *logcon;

void console_hwinit(console_t *c)
{
	logcon = c;
}

static void logchar(char ch)
{
	if (ringbuf_put(&logring, ch))
		return;

	/* ring buffer is full, discard characters until end-of-line */
	int discard;
	do {
		discard = ringbuf_get(&logring);
	} while (discard != -1 && discard != '\n');

	ringbuf_put(&logring, ch);
}

int _write(int file, char *ptr, int len)
{
	if (file == 1 || file == 2) {
		for (int i=0; i<len; i++)
			logchar(ptr[i]);
		return 0;
	}

	errno = EIO;
	return -1;
}
