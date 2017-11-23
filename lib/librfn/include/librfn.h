/*
 * librfn.h
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

#ifndef RF_LIBRFN_H_
#define RF_LIBRFN_H_

/*!
\mainpage

librfn is a collection of small library utilities collected whilst writing
"stuff" in C and wondering why I was writing the same type of code over and
over again.

It is neither organised nor comprehensive. Instead it is a pick 'n choose
library. If you like the code (and the license) use it. Either by copying the
raw C files into your application or, perhaps better, but integrating librfn
into your git project as either a submodule or subtree.

librfn is free software licensed under the GNU Lesser General Public License,
v3 or later.

Source code can be downloaded from https://github.com/daniel-thompson/librfn

*/

#ifdef __cplusplus
extern "C" {
#endif

#include "librfn/atomic.h"
#include "librfn/benchmark.h"
#include "librfn/bitops.h"
#include "librfn/console.h"
#include "librfn/enum.h"
#include "librfn/fibre.h"
#include "librfn/fixed.h"
#include "librfn/fuzz.h"
#include "librfn/hex.h"
#include "librfn/list.h"
#include "librfn/messageq.h"
#include "librfn/mlog.h"
#include "librfn/pack.h"
#include "librfn/protothreads.h"
#include "librfn/rand.h"
#include "librfn/regdump.h"
#include "librfn/ringbuf.h"
#include "librfn/stats.h"
#include "librfn/string.h"
#include "librfn/time.h"
#include "librfn/wavheader.h"
#include "librfn/util.h"

#ifdef __cplusplus
}
#endif

#endif // RF_LIBRFN_H_
