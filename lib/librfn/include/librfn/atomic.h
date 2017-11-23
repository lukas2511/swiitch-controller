/*
 * atomic.h
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

/*!
 * \brief Fallback implementation of C11 atomic operations.
 *
 * These macros a rough 'n ready implementation of C11 atomic operations based
 * upon GNU C extensions. This is useful for gcc 4.7 which does not implement
 * C11 atomics.
 */

#ifndef RF_ATOMIC_H_
#define RF_ATOMIC_H_

#include <stdint.h>

/*
 * Atomic is deliberately not presented as a doxygen module. It should be
 * considered as a bit of portability glue, not a module.
 */

#ifdef __STDC_NO_ATOMICS__

#define _Atomic

#define ATOMIC_VAR_INIT(c) c

#define atomic_init(p, c) do { *(p) = (c); } while(0)

#define memory_order_relaxed __ATOMIC_RELAXED
#define memory_order_consume __ATOMIC_CONSUME
#define memory_order_acquire __ATOMIC_ACQUIRE
#define memory_order_release __ATOMIC_RELEASE
#define memory_order_acq_rel __ATOMIC_ACQ_REL
#define memory_order_seq_cst __ATOMIC_SEQ_CST

#define atomic_thread_fence(order) \
	__atomic_thread_fence(order)
#define atomic_signal_fence(order) \
	__atomic_signal_fence(order)

typedef _Bool atomic_flag;
typedef char atomic_char;
typedef signed char atomic_schar;
typedef unsigned char atomic_uchar;
typedef int atomic_int;
typedef unsigned int atomic_uint;

#define atomic_store(object, desired) \
	__atomic_store_n(object, desired, __ATOMIC_SEQ_CST)
#define atomic_store_explicit(object, desired, order) \
	__atomic_store_n(object, desired, order)

#define atomic_load(object) \
	__atomic_load_n(object, __ATOMIC_SEQ_CST)
#define atomic_load_explicit(object, order) \
	__atomic_load_n(object, order)

#define atomic_exchange(object, desired) \
	__atomic_exchange_n(object, desired, __ATOMIC_SEQ_CST)
#define atomic_exchange_explicit(object, desired, order) \
	__atomic_exchange_n(object, desired, order)

#define atomic_compare_exchange_strong(object, expected, desired) \
	__atomic_compare_exchange_n(object, expected, desired, false, \
			__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define atomic_compare_exchange_strong_explicit(object, expected, desired, success, failure) \
		__atomic_compare_exchange_n(object, expected, desired, false, \
				success, failure)

#define atomic_compare_exchange_weak(object, expected, desired) \
		__atomic_compare_exchange_n(object, expected, desired, true, \
				__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define atomic_compare_exchange_weak_explicit(object, expected, desired, success, failure) \
		__atomic_compare_exchange_n(object, expected, desired, true, \
				success, failure)

#define atomic_fetch_add(object, operand) \
	__atomic_fetch_add(object, operand, __ATOMIC_SEQ_CST)
#define atomic_fetch_add_explicit(object, operand, order) \
	_atomic_fetch_add(object, operand, order)

#define atomic_fetch_sub(object, operand) \
	__atomic_fetch_sub(object, operand, __ATOMIC_SEQ_CST)
#define atomic_fetch_sub_explicit(object, operand, order) \
	__atomic_fetch_sub(object, operand, order)

#define atomic_fetch_or(object, operand) \
	__atomic_fetch_or(object, operand, __ATOMIC_SEQ_CST)
#define atomic_fetch_or_explicit(object, operand, order) \
	__atomic_fetch_or(object, operand, order)

#define atomic_fetch_xor(object, operand) \
	__atomic_fetch_xor(object, operand, __ATOMIC_SEQ_CST)
#define atomic_fetch_xor_explicit(object, operand, order) \
	__atomic_fetch_xor(object, operand, order)

#define atomic_fetch_and(object, operand) \
	__atomic_fetch_and(object, operand, __ATOMIC_SEQ_CST)
#define atomic_fetch_and_explicit(object, operand, order) \
	__atomic_fetch_and(object, operand, order)

#define atomic_flag_test_and_set(object) \
	__atomic_test_and_set(object, __ATOMIC_SEQ_CST)
#define atomic_flag_test_and_set_explicit(object, order) \
	__atomic_test_and_set(object, order)

#define atomic_flag_clear(object) \
	__atomic_clear(object, __ATOMIC_SEQ_CST)
#define atomic_flag_clear_explicit(object, order) \
	__atomic_clear(object, order)

#else /* __STDC_NO_ATOMICS__ */

#ifndef __cplusplus
#include <stdatomic.h>
#else
/*
 * libgcc's stdatomic.h (as of 4.9) doesn't work from C++. This workaround is a
 * hack and isn't anything like as portable as it looks (because it confuses
 * size and alignment). However for most systems it is enough to allow C++ to
 * pack structures containing atomics the same way as the C compiler.  This
 * makes it possible to use APIs linked to the structures from C++.
 */
typedef char atomic_uchar __attribute__((aligned(SIZEOF_ATOMIC_UCHAR)));
typedef unsigned int atomic_uint __attribute__((aligned(SIZEOF_ATOMIC_UINT)));
#endif /* __cplusplus */

#endif /* __STDC_NO_ATOMICS__ */

#endif // RF_ATOMIC_H_
