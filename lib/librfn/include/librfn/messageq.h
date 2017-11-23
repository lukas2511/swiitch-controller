/*
 * messageq.h
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

#ifndef RF_MESSAGEQ_H_
#define RF_MESSAGEQ_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "atomic.h"

/*!
 * \defgroup librfn_messageq Message queue
 *
 * \brief Lockless message queue implementation using C11 atomic operations
 *
 * This is a lightweight memory based message queue capable of passing
 * arbitrarily sized messages providing the size can be specified up front
 * and the memory pre-allocated.
 *
 * The message queue is thread safe for one-to-one and many-to-one messaging.
 * It cannot be used to manage multiple receiver threads without additional
 * external locking.
 *
 * \note The message queue implementation uses a bitfield to track state.
 *       For this reasons it cannot manage queues deeper than 32 messages.
 * @{
 */

typedef struct {
	char *basep;
	uint16_t msg_len;
	unsigned char queue_len;

	atomic_uchar num_free;
	atomic_uchar sendp;
	atomic_uint full_flags;

	unsigned char receivep;

} messageq_t;

#define MESSAGEQ_VAR_INIT(basep, base_len, msg_len) \
	{ \
		(char *) (basep), \
		(msg_len), \
		((base_len) / (msg_len)), \
		ATOMIC_VAR_INIT(((base_len) / (msg_len))), \
		ATOMIC_VAR_INIT(0), \
		ATOMIC_VAR_INIT(0), \
		0 \
	}

void messageq_init(messageq_t *mq, void *basep, size_t base_len, size_t msg_len);

void *messageq_claim(messageq_t *mq);
void messageq_send(messageq_t *mq, void *msg);
void *messageq_receive(messageq_t *mq);
void messageq_release(messageq_t *mq, void *msg);

static inline bool messageq_empty(messageq_t *mq)
{
	return 0 == (atomic_load(&mq->full_flags) & (1 << mq->receivep));
}

/*! @} */
#endif // RF_MESSAGEQ_H_
