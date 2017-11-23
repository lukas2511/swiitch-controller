/*
 * string.h
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2012-2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef RF_STRING_H_
#define RF_STRING_H_

#include <stdarg.h>
#include <stdlib.h>

/*!
 * \defgroup librfn_string String manipulation
 *
 * \brief Various string manipulation routines.
 *
 * This module places a heavy emphasis on strdup() and xstrdup() based
 * dynamic memory management techniques.
 *
 * @{
 */

char *xstrdup(const char *s);

char *strdup_join(const char *head, const char *tail);
char *xstrdup_join(const char *head, const char *tail);

char *strtolower(char *s);
char *strdup_tolower(const char *s);
char *xstrdup_tolower(const char *s);

char *strtoupper(char *s);
char *strdup_toupper(const char *s);
char *xstrdup_toupper(const char *s);

char *strdup_printf(const char *format, ...);
char *strdup_vprintf(const char *format, va_list ap);
char *xstrdup_printf(const char *format, ...);
char *xstrdup_vprintf(const char *format, va_list ap);

/*! @} */
#endif // RF_STRING_H_
