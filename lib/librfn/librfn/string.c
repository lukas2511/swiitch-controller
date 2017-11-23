/*
 * string.c
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

static char *xtest(char *p)
{
	if (!p)
		rf_internal_out_of_memory();

	return p;
}

char *xstrdup(const char *s)
{
	return xtest(strdup(s));
}

char *strdup_join(const char *head, const char *tail)
{
	return strdup_printf("%s%s", head, tail);
}

char *xstrdup_join(const char *head, const char *tail)
{
	return xtest(strdup_join(head, tail));
}

char *strtolower(char *s)
{
	// did *you* know that for the code to be (portably) correct you have
	// to force the argument of tolower (and indeed of any of the ctype.h
	// functions) into an unsigned type before conversion?
	//
	// I didn't until -Wall (and newlib) told me...
	for (char *p = s; *p != '\0'; p++)
		*p = tolower((unsigned char) *p);

	return s;
}

char *strdup_tolower(const char *s)
{
	char *t = strdup(s);
	return (t ? strtolower(t) : NULL);
}

char *xstrdup_tolower(const char *s)
{
	return xtest(strdup_tolower(s));
}

char *strtoupper(char *s)
{
	// to understand the cast see strtolower...
	for (char *p = s; *p != '\0'; p++)
		*p = toupper((unsigned char) *p);

	return s;
}

char *strdup_toupper(const char *s)
{
	char *t = strdup(s);
	return (t ? strtoupper(t) : NULL);
}

char *xstrdup_toupper(const char *s)
{
	return xtest(strdup_toupper(s));
}

char *strdup_printf(const char *fmt, ...)
{
	char *str;
	va_list ap;

	va_start(ap, fmt);
	str = strdup_vprintf(fmt, ap);
	va_end(ap);

	return str;
}

char *strdup_vprintf(const char *fmt, va_list ap)
{
	char *str;
	int len;
	va_list nap;

	va_copy(nap, ap);
	len = vsnprintf(NULL, 0, fmt, nap);
	va_end(nap);

	str = malloc(len+1);
	if (str)
		vsprintf(str, fmt, ap);

	return str;
}

char *xstrdup_printf(const char *fmt, ...)
{
	char *str;
	va_list ap;

	va_start(ap, fmt);
	str = xstrdup_vprintf(fmt, ap);
	va_end(ap);

	return str;
}

char *xstrdup_vprintf(const char *fmt, va_list ap)
{
	char *str;
	int len;
	va_list nap;

	va_copy(nap, ap);
	len = vsnprintf(NULL, 0, fmt, nap);
	va_end(nap);

	str = xmalloc(len+1);
	if (str)
		vsprintf(str, fmt, ap);

	return str;
}
