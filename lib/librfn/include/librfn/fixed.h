/*
 * fixed.h
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

#ifndef RF_FIXED_H_
#define RF_FIXED_H_

#include <stdint.h>

/*! \brief Q19 fixed point type (or a float if HAVE_FPU is set)
 *
 * The compiler cannot do any meaningful type checking because all
 * fixed pointer values ultimately have integer type. Thus its use
 * is essentially a form of commenting.
 */
typedef int32_t ttq19_t;

typedef int64_t ttq38_t;



#endif // RF_FIXED_H_
