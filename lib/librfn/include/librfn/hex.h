/*
 * hex.h
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

#ifndef RF_HEX_H_
#define RF_HEX_H_

#include <stdio.h>
#include <stdlib.h>

/*!
 * \defgroup librfn_hex Hex dump
 *
 * \brief Hexidecimal dump to stdout or a FILE.
 *
 * @{
 */

int hex_dump(unsigned char *p, size_t sz);
int hex_dump_to_file(FILE *f, unsigned char *p, size_t sz);

int hex_get_byte(const char *s, const char **p);

/*! @} */
#endif // RF_HEX_H_
