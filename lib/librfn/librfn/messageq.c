/*
 * messageq.c
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

#include "librfn/messageq.h"

#include <assert.h>
#include <string.h>

#include "librfn/util.h"

void messageq_init(messageq_t *mq, void *basep, size_t base_len, size_t msg_len)
{
	memset(mq, 0, sizeof(*mq));

	mq->basep = basep;
	mq->msg_len = msg_len;
	mq->queue_len = base_len / msg_len;
	atomic_store(&mq->num_free, base_len / msg_len);
}

void *messageq_claim(messageq_t *mq)
{
	/* get permission to allocate a message */
	int num_free = atomic_fetch_sub(&mq->num_free, 1);
	if (num_free <= 0) {
		atomic_fetch_add(&mq->num_free, 1);
		return NULL;
	}

	/* find out the address that will be allocated */
	unsigned char sendp = atomic_load(&mq->sendp);
	unsigned char newsendp;
	do {
		newsendp = (sendp >= (mq->queue_len-1) ? 0 : sendp+1);
	} while(!atomic_compare_exchange_weak(&mq->sendp, &sendp, newsendp));

	return mq->basep + (sendp * mq->msg_len);
}

void messageq_send(messageq_t *mq, void *msg)
{
	unsigned int offset = (((char *) msg) - mq->basep);
	unsigned int sendp = offset / mq->msg_len;
	atomic_fetch_or(&mq->full_flags, (1 << sendp));
}

void *messageq_receive(messageq_t *mq)
{
	unsigned int receivep = mq->receivep;
	unsigned int full_flags = atomic_fetch_and(
			&mq->full_flags, ~(1 << receivep));

	if (0 == (full_flags & (1 << receivep)))
		return NULL;

	mq->receivep =
	    (receivep >= (unsigned int)(mq->queue_len - 1) ? 0 : receivep + 1);

	return mq->basep + (receivep * mq->msg_len);

}

void messageq_release(messageq_t *mq, void *msg)
{
	/* msg is part of the API to allow automatic checking that messages are
	 * always released (and therefore received) in strict order. However
	 * that is not yet implemented.
	 */
	(void)msg;
	atomic_fetch_add(&mq->num_free, 1);
}
