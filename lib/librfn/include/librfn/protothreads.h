/*
 * protothreads.h
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2013-2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef RF_PROTOTHREADS_H_
#define RF_PROTOTHREADS_H_

#include <assert.h>
#include <stdint.h>

/*!
 * \defgroup librfn_protothreads Protothreads
 *
 * \brief Cooperative multi-tasking without context switching
 *
 * Protothreads rely on the normal function call mechanism to switch
 * contexts, however macros provides by the protothread implementation
 * result in execution continuing from the point the thread yielded
 * the processor.
 *
 * This very simple example does not require a scheduler. Instead the blink
 * function can be called from a polling loop (or systick interrupt) along
 * with all system activity.
 *
 * \code
 * int blink_led(void) {
 *	static pt_state_t ps;
 *	static uint64_t t;
 *
 *	PT_INIT(&ps);
 *
 *      t = time64_now();
 *	while (true) {
 *		led_toggle();
 *		t += 250000;
 *
 *		PT_WAIT_UNTIL(time64_now() > t);
 *	}
 *
 *	PT_END();
 * }
 * \endcode
 *
 * \note In dynamic environments where the blink_led() routine is used to
 *       implement more than one thread then the pt_state_t and the time, t,
 *       must be passed in as an argument (usually as part of a context
 *       structure) rather than being static variables.
 *
 * Like all protothread implementations this work is inspired by the paper
 * "Protothreads: Simplifying Event-Driven Programming of Memory-Constrained
 * Embedded Systems" by Dunkels et al.
 *
 * This implementation uses a similar interface as that proposed by Dunkels et
 * al but with sufficient changes to make them incompatible:
 *
 * 1. Stronger distinction between yielding and waiting. This allows a
 *    scheduler to handle the cases differently. The fibre scheduler included
 *    in this library observes this distinction. We also eliminate the local
 *    continuation macros concept allowing an implementation of PT_YIELD()
 *    with no state variable.
 *
 * 2. PT_INIT() saves the protothread pointer to simplify the usage of the other
 *    macros.
 *
 * 3. The use of the provided pt_t type is optional. Any integer type large
 *    enough not to overflow when __LINE__ is assigned to it is sufficient.
 *
 * @{
 */

typedef enum {
	PT_YIELDED,
	PT_WAITING,
	PT_EXITED
} pt_state_t;

typedef uint16_t pt_t;

#define PT_INIT(pt)                                                            \
	do {                                                                   \
		*(pt) = 0;                                                     \
	} while (0)

#define PT_BEGIN(pt)                                                           \
	{                                                                      \
		pt_t *missing_PT_BEGIN = pt;                                   \
		(void) missing_PT_BEGIN;                                       \
		switch (*missing_PT_BEGIN) {                                   \
		case 0:

#define PT_END()                                                               \
			break;                                                 \
		default:                                                       \
			assert(0);                                             \
		}                                                              \
	}                                                                      \
	return PT_EXITED

#define PT_WAIT()                                                              \
	do {                                                                   \
		*missing_PT_BEGIN = __LINE__;                                  \
		return PT_WAITING;                                             \
	case __LINE__:                                                         \
		;                                                              \
	} while (0)

#define PT_WAIT_UNTIL(c)                                                       \
	do {                                                                   \
		*missing_PT_BEGIN = __LINE__;                                  \
	case __LINE__:                                                         \
		if (!(c))                                                      \
			return PT_WAITING;                                     \
	} while (0)

#define PT_YIELD()                                                             \
	do {                                                                   \
		*missing_PT_BEGIN = __LINE__;                                  \
		return PT_YIELDED;                                             \
	case __LINE__:                                                         \
		;                                                              \
	} while (0)

#define PT_EXIT() \
	return PT_EXITED

/*!
 * \brief Conditional exit.
 *
 * Mostly used to simplify error paths.
 */
#define PT_EXIT_ON(x)                                                          \
	do {                                                                   \
		if (x)                                                         \
			return PT_EXITED;                                      \
	} while (0)

/*!
 * \brief Call a child thread.
 */
#define PT_SPAWN(child, thread)                                                \
	do {                                                                   \
		pt_state_t ptres;                                              \
		PT_INIT(child);                                                \
		*missing_PT_BEGIN = __LINE__;                                  \
	case __LINE__:                                                         \
		ptres = (thread);                                              \
		if (ptres != PT_EXITED)                                        \
			return ptres;                                          \
	} while (0)

/*!
 * \brief Call a child thread synchronously.
 *
 * Run a protothread without ever yielding. This macro is intended to allow
 * protothread library code to be executed in a non-protothreaded environment
 * (for example during initialization routines).
 */
#define PT_CALL(child, thread)                                                 \
	do {                                                                   \
		PT_INIT(child);                                                \
		while (PT_EXITED != (thread))                                  \
			;                                                      \
	} while (0)

/*! @} */
#endif // RF_PROTOTHREADS_H_
