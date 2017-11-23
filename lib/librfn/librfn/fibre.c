/*
 * fibre.c
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

#include "librfn/fibre.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "librfn/atomic.h"
#include "librfn/list.h"
#include "librfn/messageq.h"
#include "librfn/util.h"

static fibre_t *atomic_runq_buf[8];

static struct {
	fibre_t *current;
	fibre_state_t state;
	uint32_t now;

	list_t runq;
	messageq_t atomic_runq;
	list_t timerq;

	atomic_uint taint_flags;
} kernel = {
	.runq = LIST_VAR_INIT,
	.atomic_runq = MESSAGEQ_VAR_INIT(
			atomic_runq_buf,
			sizeof(atomic_runq_buf), sizeof(atomic_runq_buf[0])),
	.timerq = LIST_VAR_INIT
};

static void handle_atomic_runq(void)
{
	fibre_t **f;

	while (NULL != (f = messageq_receive(&kernel.atomic_runq))) {
		fibre_run(*f);
		messageq_release(&kernel.atomic_runq, f);
	}
}

static void handle_timerq(void)
{
	list_iterator_t iter;

	list_node_t *node = list_iterate(&kernel.timerq, &iter);
	fibre_t *timeout_fibre = containerof(node, fibre_t, link);
	while (NULL != node &&
			cyclecmp32(timeout_fibre->duetime, kernel.now) <= 0) {
		node = list_iterator_remove(&iter);
		list_insert(&kernel.runq, &timeout_fibre->link);
		timeout_fibre = containerof(node, fibre_t, link);
	}

}

static fibre_t *get_next_task(void)
{
	list_node_t *node = list_extract(&kernel.runq);
	if (!node)
		return NULL;

	return containerof(node, fibre_t, link);
}

static void update_current_state(void)
{
	kernel.current->state = kernel.state;

	switch (kernel.state) {
	case FIBRE_STATE_YIELDED:
		fibre_run(kernel.current);
		break;
	case FIBRE_STATE_EXITED:
		PT_INIT(&kernel.current->priv);
		break;
	case FIBRE_STATE_WAITING:
		break;
	default:
		assert(0);
		break;
	}

	// preserve kernel.current since we permit fibre_self() to be called
	// from the main loop in order to implement task accounting.
}

static uint32_t get_next_wakeup(void)
{
	if (!messageq_empty(&kernel.atomic_runq) || !list_empty(&kernel.runq))
		return kernel.now;

	if (list_empty(&kernel.timerq))
		return kernel.now + FIBRE_UNBOUNDED_SLEEP;

	fibre_t *fibre = containerof(list_peek(&kernel.timerq), fibre_t, link);
	return fibre->duetime;
}

static void add_taint(char id)
{
	id -= 'A';
	assert(id < 8*sizeof(kernel.taint_flags));
	atomic_fetch_or(&kernel.taint_flags, 1 << id);
}

static int duetime_cmp(list_node_t *n1, list_node_t *n2)
{
	fibre_t *f1 = containerof(n1, fibre_t, link);
	fibre_t *f2 = containerof(n2, fibre_t, link);

	return f1->duetime - f2->duetime;
}

fibre_t *fibre_self()
{
	return kernel.current;
}

uint32_t fibre_scheduler_next(uint32_t time)
{
	kernel.now = time;

	/*
	 * When we have a single fibre yielding to itself we can create a
	 * fast path by skipping scheduler updates. This allows processor
	 * intensive work to be harmed as little as possible even when the
	 * fibre they run in seeks to cooperate with other fibres.
	 */
	if (kernel.state != FIBRE_STATE_YIELDED ||
	    !list_empty(&kernel.runq) ||
	    !list_empty(&kernel.timerq) ||
	    !messageq_empty(&kernel.atomic_runq)) {
		handle_atomic_runq();
		if (kernel.current)
			update_current_state();
		handle_timerq();
		kernel.current = get_next_task();
	}

	if (kernel.current) {
		kernel.state = kernel.current->fn(kernel.current);
		if (kernel.state == FIBRE_STATE_YIELDED)
			return kernel.now;
	}

	return get_next_wakeup();
}

void fibre_init(fibre_t *f, fibre_entrypoint_t *fn)
{
	memset(f, 0, sizeof(*f));

	f->fn = fn;
	// TODO: list_node_init
	//list_node_init(&f->link);
}

void fibre_run(fibre_t *f)
{
	handle_atomic_runq();

	if (!list_contains(&kernel.runq, &f->link, NULL)) {
		(void) list_remove(&kernel.timerq, &f->link);
		list_insert(&kernel.runq, &f->link);
	}
}

bool fibre_run_atomic(fibre_t *f)
{
	fibre_t **queued_fibre = messageq_claim(&kernel.atomic_runq);
	if (!queued_fibre) {
		add_taint('A');
		return false;
	}

	*queued_fibre = f;
	messageq_send(&kernel.atomic_runq, queued_fibre);
	return true;
}

bool fibre_kill(fibre_t *f)
{
	bool res = false;

	handle_atomic_runq();

	res |= list_remove(&kernel.runq, &f->link);
	res |= list_remove(&kernel.timerq, &f->link);

	return res;
}

bool fibre_timeout(uint32_t duetime)
{
	if (cyclecmp32(duetime, kernel.now) <= 0)
		return true;

	kernel.current->duetime = duetime;
	if (!list_contains(&kernel.runq, &kernel.current->link, NULL))
		list_insert_sorted(&kernel.timerq, &kernel.current->link, duetime_cmp);
	return false;
}

void fibre_eventq_init(fibre_eventq_t *evtq, fibre_entrypoint_t *fn,
		void *basep, size_t base_len, size_t msg_len)
{
	fibre_init(&evtq->fibre, fn);
	messageq_init(&evtq->eventq, basep, base_len, msg_len);
}

void *fibre_eventq_claim(fibre_eventq_t *evtq)
{
	void *evtp = messageq_claim(&evtq->eventq);
	if (!evtp)
		add_taint('E');
	return evtp;
}

bool fibre_eventq_send(fibre_eventq_t *evtq, void *evtp)
{
	messageq_send(&evtq->eventq, evtp);
	return fibre_run_atomic(&evtq->fibre);
}

bool fibre_eventq_empty(fibre_eventq_t *evtq)
{
	return messageq_empty(&evtq->eventq);
}

void *fibre_eventq_receive(fibre_eventq_t *evtq)
{
	return messageq_receive(&evtq->eventq);
}

void fibre_eventq_release(fibre_eventq_t *evtq, void *evtp)
{
	messageq_release(&evtq->eventq, evtp);
}
