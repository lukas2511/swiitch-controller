/*
 * ringbuf.c
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

#include "librfn/ringbuf.h"

#include <assert.h>
#include <string.h>

#include "librfn/atomic.h"
#include "librfn/util.h"

void ringbuf_init(ringbuf_t *rb, void *bufp, size_t buf_len)
{
	memset(rb, 0, sizeof(ringbuf_t));

	rb->bufp = bufp;
	rb->buf_len = buf_len;
}

/* TODO: the memory ordering in this function is needlessly aggressive */
int ringbuf_get(ringbuf_t *rb)
{
	unsigned int readi = atomic_load(&rb->readi);
	int d;

	assert(readi < rb->buf_len);

	if (readi == atomic_load(&rb->writei))
		return -1;

	d = rb->bufp[readi];
	atomic_signal_fence(memory_order_seq_cst);

	if (++readi >= rb->buf_len)
		readi -= rb->buf_len;

	atomic_store(&rb->readi, readi);
	return d;
}

/* TODO: the memory ordering in this function is needlessly aggressive */
bool ringbuf_empty(ringbuf_t *rb)
{
	return atomic_load(&rb->readi) == atomic_load(&rb->writei);
}

/* TODO: the memory ordering in this function is needlessly aggressive */
bool ringbuf_put(ringbuf_t *rb, uint8_t d)
{
	unsigned int writei = atomic_load(&rb->writei);
	unsigned int old_writei = writei;

	if (++writei >= rb->buf_len)
		writei -= rb->buf_len;
	
	if (writei == atomic_load(&rb->readi))
		return false;
	
	rb->bufp[old_writei] = d;
	atomic_signal_fence(memory_order_seq_cst);
	atomic_store(&rb->writei, writei);
	return true;
}

void ringbuf_putchar(void *rb, char c)
{
	while (!ringbuf_put(rb, c))
		;
}
