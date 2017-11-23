/*
 * mlog.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2015 Daniel Thompson <daniel@redfelineninja.org.uk> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "librfn/mlog.h"

#include <stdint.h>

#include "librfn/string.h"
#include "librfn/util.h"

struct mlog_line {
	const char *fmt;
	uintptr_t arg[3];
};

struct mlog {
	struct mlog_line line[256];
	unsigned int head;
};

static struct mlog log;

void vmlog(const char *fmt, va_list ap)
{
	unsigned int head = log.head % lengthof(log.line);
	
	log.line[head].fmt = fmt;
	log.line[head].arg[0] = va_arg(ap, uintptr_t);
	log.line[head].arg[1] = va_arg(ap, uintptr_t);
	log.line[head].arg[2] = va_arg(ap, uintptr_t);

	log.head++;
	if (log.head >= 0x7fffffff)
		log.head -= lengthof(log.line);
}

void mlog(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vmlog(fmt, ap);
	va_end(ap);
}

void vmlog_nice(const char *fmt, va_list ap)
{
	if (log.head < lengthof(log.line))
		vmlog(fmt, ap);
}

void mlog_nice(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vmlog_nice(fmt, ap);
	va_end(ap);
}

void mlog_clear(void)
{
	log.head = 0;
}

static struct mlog_line *get_line(unsigned int n)
{
	if (n >= log.head || n >= lengthof(log.line))
		return NULL;

	/* if the buffer has wrapped adjust n by the wrap point */
	if (log.head >= lengthof(log.line))
		n += log.head;

	return &log.line[n % lengthof(log.line)];
}

void mlog_dump(FILE *f)
{
	struct mlog_line *line;

	for (int i=0; (line = get_line(i)); i++)
		fprintf(f, line->fmt, line->arg[0], line->arg[1], line->arg[2]);
}

char *mlog_get_line(int n)
{
	struct mlog_line *line = get_line(n);

	if (line)
		return strdup_printf(line->fmt, line->arg[0], line->arg[1],
				     line->arg[2]);

	return NULL;
}
