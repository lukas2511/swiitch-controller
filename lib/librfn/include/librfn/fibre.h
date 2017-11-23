/*
 * fibre.h
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

#ifndef RF_FIBRE_H_
#define RF_FIBRE_H_

#include <stdint.h>

#include "list.h"
#include "messageq.h"
#include "protothreads.h"

/*!
 * \defgroup librfn_fibre Fibre
 *
 * \brief Lightweight run-to-completion task scheduler supporting protothreads.
 *
 * The fibre scheduler is an extremely lightweight run-to-completion task
 * scheduler supporting protothreads. In addition to timer and run queue
 * management it also provides support for per-fibre event queues.
 * @{
 */

typedef enum {
	FIBRE_STATE_YIELDED = PT_YIELDED,
	FIBRE_STATE_WAITING = PT_WAITING,
	FIBRE_STATE_EXITED = PT_EXITED,
	FIBRE_STATE_RUNNING,

	FIBRE_STATE_QUEUED = 0x10,

	FIBRE_STATE_TIMER_WAITING = FIBRE_STATE_WAITING | FIBRE_STATE_QUEUED,
	FIBRE_STATE_RUNNABLE = FIBRE_STATE_RUNNING | FIBRE_STATE_QUEUED
} fibre_state_t;

/*!
 * \brief An approximation of infinitely far in the future.
 *
 * This is the largest possible value that can be added to the current time
 * and still yeild a value in the future (a larger value will cause the time
 * to wrap around to be in the past).
 *
 * With the typical 1MHz clock provided by time_now(), it actually
 * represents a little over three and a half minutes.
 */
#define FIBRE_UNBOUNDED_SLEEP ((uint32_t) 0x7fffffff)

struct fibre;
typedef int fibre_entrypoint_t(struct fibre *);

/*!
 * \brief Fibre descriptor.
 */
typedef struct fibre {
	fibre_entrypoint_t *fn;
	uint16_t state;
	uint16_t priv;
	uint32_t duetime;
	list_node_t link;
} fibre_t;

/*!
 * \brief Static initializer for a fibre descriptor.
 */
#define FIBRE_VAR_INIT(fn) { (fn), 0, 0, 0, LIST_NODE_VAR_INIT }

/*!
 * \brief Fibre and eventq descriptor.
 *
 * This descriptor is a fibre and messag queue bound together so that
 * events can be passed to the fibre.
 *
 * Allows the fibre to be automatically scheduled if events are passed
 * using fibre_eventq_send().
 */
typedef struct fibre_eventq {
	fibre_t fibre;
	messageq_t eventq;
} fibre_eventq_t;

/*!
 * \brief Static initializer for a fibre and eventq descriptor.
 */
#define FIBRE_EVENTQ_VAR_INIT(fn, basep, base_len, msg_len) \
	{ \
		.fibre = FIBRE_VAR_INIT(fn), \
		.eventq = MESSAGEQ_VAR_INIT(basep, base_len, msg_len) \
	}

/*!
 * \brief Fibre aware alternative to PT_BEGIN().
 */
#define PT_BEGIN_FIBRE(f) PT_BEGIN(&((f)->priv))

/*!
 * \brief Returns the currently active fibre descriptor.
 */
fibre_t *fibre_self(void);

/*!
 * \brief Schedule the next fibre.
 *
 * This is the heart of the fibre system. Any fibre whose due time is before
 * time will be made runnable and the fibre at the head of the run queue
 * will execute. When that fibre completes the scheduler returns the time
 * of the next wakeup (which may be in the past if the run queue contains
 * active tasks).
 *
 * \note fibre_self(), when called after fibre_scheduler_next() has executed,
 *       returns the fibre that was most recently scheduled. This can be used
 *       to implement task accounting and to debug scheduling delays.
 *
 * The return value provides the time at which the next task must be scheduled
 * this allows low power modes to implemented by programming a wakeup timer and
 * sleeping until it fires.
 *
 * \warning It is possible for an interrupt service routine to make a fibre
 *          runnable between this function returning and entering a low power
 *          state. On ARM architecture the WFE instruction avoids potential
 *          races by managing an event flag which is set by reception of
 *          interrupt. On hardware without this feature additional work is
 *          required to safely enter low power states.
 *
 * \param time current time in cyclic 32-bit time (e.g. 0 is one tick
 *             after 0xffffffff)
 * \return time at which the next task must be scheduled
 */
uint32_t fibre_scheduler_next(uint32_t time);

/*!
 * \brief Dynamic initializer for a fibre descriptor.
 */
void fibre_init(fibre_t *f, fibre_entrypoint_t *fn);

/*!
 * Make a fibre runnable.
 *
 * When a fibre is made runnable it will be appended to the run queue
 * and will, eventually be executed during a call to fibre_scheduler_next().
 *
 * \warning Do not call this function from an interrupt service routine, use
 *          fibre_run_atomic() instead.
 */
void fibre_run(fibre_t *f);

/*!
 * Make a fibre runnable using only atomic operations.
 *
 * This function behaves similarly fibre_run(). It is less efficient than
 * the alternative but can be used from any calling context, including from an
 * interrupt service routine.
 */
bool fibre_run_atomic(fibre_t *f);

/*!
 * Remove a fibre from the run queue.
 */
bool fibre_kill(fibre_t *f);

/*!
 * Sleep until a timeout is reached.
 *
 * Combining this function with PT_WAIT_UNTIL() allows a fibre to sleep.
 * It will be automatically added to the run queue when the specified time
 * is reached.
 */
bool fibre_timeout(uint32_t duetime);

/*!
 * \brief Dynamic initializer for a fibre and eventq descriptor.
 */
void fibre_eventq_init(fibre_eventq_t *evtq, fibre_entrypoint_t *fn,
		void *basep, size_t base_len, size_t msg_len);

/*!
 * \brief Request memory resources to send an event to a fibre.
 *
 * \note If the fibre's event queue is full the kernel will automtically
 *       be tainted (the 'E' bit will be set).
 */
void *fibre_eventq_claim(fibre_eventq_t *evtq);

/*!
 * \brief Send an event to a fibre.
 *
 * If the fibre is not already runnable then it will be added to the run queue.
 */
bool fibre_eventq_send(fibre_eventq_t *evtq, void *evtp);

/*!
 * \brief Return true if the fibre's event queue is empty.
 */
bool fibre_eventq_empty(fibre_eventq_t *evtq);

/*!
 * \brief Recevied a message previously send to the fibre.
 *
 * This function can be used by a fibre to receive an event.
 *
 * \warning This function should only be called by the fibre bound to the
 *          event queue and must not be called by an interrupt service routine.
 */
void *fibre_eventq_receive(fibre_eventq_t *evtq);

/*!
 * \brief Release a message previously received by a fibre.
 */
void fibre_eventq_release(fibre_eventq_t *evtq, void *evtp);

/*!
 * Enter the scheduler main loop.
 *
 * This function will dispatch threads when they become runnable and
 * may attempt to conserve power by going idle in some implementation
 * defined manner.
 *
 * An implementation of this function must be provided for each execution
 * environment. librfn only comes with an implementation for a small
 * subset of environments and may have to be provided by the library user.
 * POSIX environments are, however, already supported to allow the
 * demonstation programs to work.
 */
void fibre_scheduler_main_loop(void);

/*! @} */

#endif // RF_FIBRE_H_
