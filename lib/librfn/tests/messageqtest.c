/*
 * messageqtest.c
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

#undef NDEBUG

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <librfn.h>

int queue_buf[3];
messageq_t queue = MESSAGEQ_VAR_INIT(
		queue_buf, sizeof(queue_buf), sizeof(queue_buf[0]));


int main()
{
	messageq_t myqueue;

	/*
	 * The messageq_t data structure is intended to be safe to *send*
	 * messages from multiple threads (although only one receiver thread
	 * is permitted). However at present these tests only check for
	 * sequential correctness.
	 */

	/* prove the equivalence of the initializer and the init fn */
	messageq_init(&myqueue,
			queue_buf, sizeof(queue_buf), sizeof(queue_buf[0]));
	verify(0 == memcmp(&queue, &myqueue, sizeof(queue)));

	/* simple claim, send, receive, release loop */
	for (int i=0; i<lengthof(queue_buf)*3; i++) {
		int *sendp = messageq_claim(&queue);
		verify(queue_buf + (i%lengthof(queue_buf)) == sendp);

		*sendp = i;
		messageq_send(&queue, sendp);

		int *receivep = messageq_receive(&queue);
		verify(sendp == receivep);
		verify(*receivep == i);

		messageq_release(&queue, receivep);
	}

	/* zealous claim with in-order sends and greedy receive */
	verify(queue_buf+0 == messageq_claim(&queue));
	verify(queue_buf+1 == messageq_claim(&queue));
	verify(queue_buf+2 == messageq_claim(&queue));
	verify(NULL == messageq_claim(&queue));

	messageq_send(&queue, queue_buf+0);
        verify(queue_buf+0 == messageq_receive(&queue));
        verify(NULL == messageq_receive(&queue));
        messageq_release(&queue, queue_buf+0);

        messageq_send(&queue, queue_buf+1);
        verify(queue_buf+1 == messageq_receive(&queue));
        verify(NULL == messageq_receive(&queue));
        messageq_release(&queue, queue_buf+1);

        messageq_send(&queue, queue_buf+2);
        verify(queue_buf+2 == messageq_receive(&queue));
        verify(NULL == messageq_receive(&queue));
        messageq_release(&queue, queue_buf+2);

	/* zealous claim with out-of-order sends and greedy receive */
	verify(queue_buf+0 == messageq_claim(&queue));
	verify(queue_buf+1 == messageq_claim(&queue));
	verify(queue_buf+2 == messageq_claim(&queue));
	verify(NULL == messageq_claim(&queue));

	messageq_send(&queue, queue_buf+2);
        verify(NULL == messageq_receive(&queue));

        messageq_send(&queue, queue_buf+1);
        verify(NULL == messageq_receive(&queue));

        messageq_send(&queue, queue_buf+0);
        verify(queue_buf+0 == messageq_receive(&queue));
        messageq_release(&queue, queue_buf+0);
        verify(queue_buf+1 == messageq_receive(&queue));
        messageq_release(&queue, queue_buf+1);
        verify(queue_buf+2 == messageq_receive(&queue));
        messageq_release(&queue, queue_buf+2);
        verify(NULL == messageq_receive(&queue));

	/* emptyness tests */
	verify(true == messageq_empty(&queue));

	verify(queue_buf+0 == messageq_claim(&queue));
	verify(true == messageq_empty(&queue));
	verify(queue_buf+1 == messageq_claim(&queue));
	verify(true == messageq_empty(&queue));
	verify(queue_buf+2 == messageq_claim(&queue));
	verify(true == messageq_empty(&queue));

	messageq_send(&queue, queue_buf+0);
	verify(false == messageq_empty(&queue));
	messageq_send(&queue, queue_buf+1);
	verify(false == messageq_empty(&queue));
	messageq_send(&queue, queue_buf+2);
	verify(false == messageq_empty(&queue));

        verify(queue_buf+0 == messageq_receive(&queue));
	verify(false == messageq_empty(&queue));
        messageq_release(&queue, queue_buf+0);
	verify(false == messageq_empty(&queue));
        verify(queue_buf+1 == messageq_receive(&queue));
	verify(false == messageq_empty(&queue));
        messageq_release(&queue, queue_buf+1);
	verify(false == messageq_empty(&queue));
        verify(queue_buf+2 == messageq_receive(&queue));
	verify(true == messageq_empty(&queue));
        messageq_release(&queue, queue_buf+2);
	verify(true == messageq_empty(&queue));

	return 0;
}
