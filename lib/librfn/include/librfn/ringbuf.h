/*
 * ringbuf.h
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef RF_RINGBUF_H_
#define RF_RINGBUF_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "atomic.h"

/*!
 * \defgroup librfn_ringbuf Ring buffer
 *
 * \brief Lockless byte-based ring buffer using C11 atomic operations.
 *
 * The message queue is thread safe (and SMP safe) only for one-to-one
 * messaging. It cannot be used with multiple sender threads nor
 * multiple receiver threads without additional locking.
 *
 * @{
 */

/*!
 * \brief Ring buffer descriptor.
 */
typedef struct {
	uint8_t *bufp;
	size_t buf_len;
	atomic_uint readi;
	atomic_uint writei;
} ringbuf_t;

/*!
 * \brief Static initializer for a ring buffer descriptor.
 */
#define RINGBUF_VAR_INIT(bufp, buf_len) \
	{ \
		(uint8_t *) (bufp), \
		(buf_len), \
		ATOMIC_VAR_INIT(0), \
		ATOMIC_VAR_INIT(0) \
	}

/*!
 * \brief Runtime initializer for a ring buffer descriptor.
 */
void ringbuf_init(ringbuf_t *rb, void *bufp, size_t buf_len);

/*!
 * \brief Extract a byte from the ring buffer.
 *
 * \returns Unsigned byte on success, otherwise -1.
 */
int ringbuf_get(ringbuf_t *rb);

/*!
 * \brief Test whether the ring buffer contains any data.
 *
 * \returns true if data is present, otherwise false.
 */
bool ringbuf_empty(ringbuf_t *rb);

/*!
 * \brief Insert a byte into the ring buffer.
 */
bool ringbuf_put(ringbuf_t *rb, uint8_t d);

/*!
 * \brief Insert a character into the ring buffer.
 *
 * This function is intended to be use from callback functions. For this reason
 * the ringbuffer is passed as a void pointer (i.e. user data) and there is no
 * return value.
 *
 * \warning This function busy waits until there is space in the ring buffer.
 *          This leaves the system at risk of deadlock if this function is
 *          called from a calling context that can pre-empt the consumer.
 *          For example if the data is consumed by an interrupt handler then
 *          this function should not be called only from higher (or equal)
 *          priority interrupt handler.
 */
void ringbuf_putchar(void *rb, char c);

/*! @} */
#endif // RF_RINGBUF_H_
