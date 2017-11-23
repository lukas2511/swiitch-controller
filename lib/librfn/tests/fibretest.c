/*
 * fibretest.c
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

#include <librfn.h>

typedef struct {
	uint32_t time;
	uint32_t max_time;
	uint32_t step;
	fibre_t fibre;
} sleep_fibre_t;

int sleep_fibre(fibre_t *f)
{
	sleep_fibre_t *s = containerof(f, sleep_fibre_t, fibre);

	PT_BEGIN_FIBRE(f);

	while (s->time < s->max_time) {
		s->time += s->step;
		PT_WAIT_UNTIL(fibre_timeout(s->time));
	}

	PT_END();
}

typedef struct {
	uint32_t count;
	uint32_t max_count;
	fibre_t fibre;
} yield_fibre_t;

int yield_fibre(fibre_t *f)
{
	yield_fibre_t *s = containerof(f, yield_fibre_t, fibre);

	PT_BEGIN_FIBRE(f);

	while (s->count < s->max_count) {
		s->count++;
		PT_YIELD();
	}

	PT_END();
}

/* identical behaviour as a yield_fibre but differently realized */
int exit_fibre(fibre_t *f)
{
	yield_fibre_t *s = containerof(f, yield_fibre_t, fibre);

	PT_BEGIN_FIBRE(f);

	if (s->count < s->max_count) {
		s->count++;
		fibre_run(f);
	}

	PT_END();
}

/* identical behaviour as a yield_fibre but differently realized */
int atomic_fibre(fibre_t *f)
{
	yield_fibre_t *s = containerof(f, yield_fibre_t, fibre);

	PT_BEGIN_FIBRE(f);

	if (s->count < s->max_count) {
		s->count++;
		fibre_run_atomic(f);
	}

	PT_END();
}


typedef struct {
	int id;
} event_descriptor_t;

typedef struct {
	event_descriptor_t last_event;
	fibre_eventq_t evtq;
} eventq_t;

int event_handler(fibre_t *f)
{
	fibre_eventq_t *fevtq = containerof(f, fibre_eventq_t, fibre);
	eventq_t *evtq = containerof(fevtq, eventq_t, evtq);

	PT_BEGIN_FIBRE(f);

	while (true) {
		event_descriptor_t *evt;

		PT_WAIT_UNTIL(NULL != (evt = fibre_eventq_receive(fevtq)));
		evtq->last_event = *evt;
		fibre_eventq_release(fevtq, evt);
	}

	PT_END();
}


/*
 * This is a somewhat unfocused test of general scheduler behavior.
 */
static void basic_test()
{
	static sleep_fibre_t sleeper = {
			.max_time = 50,
			.step = 10,
			.fibre = FIBRE_VAR_INIT(sleep_fibre)
	};

	static yield_fibre_t yielder = {
			.count = 0,
			.max_count = 5,
			.fibre = FIBRE_VAR_INIT(yield_fibre)
	};

	static event_descriptor_t events[8];
	static eventq_t handler = {
			.evtq = FIBRE_EVENTQ_VAR_INIT(event_handler,
					events, sizeof(events), sizeof(events[0]))
	};

	fibre_run(&sleeper.fibre);

	/* only one fibre is running, it sleeps making system go idle */
	verify( 10 == fibre_scheduler_next(  0) && fibre_self() == &sleeper.fibre);
	verify( 10 == fibre_scheduler_next(  5) && fibre_self() == NULL);
	verify( 10 == sleeper.time && 0 == yielder.count);
	verify( 20 == fibre_scheduler_next( 10) && fibre_self() == &sleeper.fibre);
	verify( 20 == sleeper.time && 0 == yielder.count);

	/* two fibres are running, one sleeps, the other yields so system
	 * remains busy
	 */
	fibre_run(&yielder.fibre);
	verify( 20 == fibre_scheduler_next( 20) && fibre_self() == &yielder.fibre); // sleeper runnable
	verify( 20 == sleeper.time && 1 == yielder.count);
	verify( 21 == fibre_scheduler_next( 21) && fibre_self() == &sleeper.fibre);
	verify( 30 == sleeper.time && 1 == yielder.count);
	verify( 22 == fibre_scheduler_next( 22) && fibre_self() == &yielder.fibre);
	verify( 30 == sleeper.time && 2 == yielder.count);
	verify( 31 == fibre_scheduler_next( 31) && fibre_self() == &yielder.fibre); // sleeper runnable
	verify( 30 == sleeper.time && 3 == yielder.count);
	verify( 42 == fibre_scheduler_next( 42) && fibre_self() == &sleeper.fibre); // sleeper loops twice
	verify( 50 == sleeper.time && 3 == yielder.count);
	verify( 44 == fibre_scheduler_next( 44) && fibre_self() == &yielder.fibre);
	verify( 50 == sleeper.time && 4 == yielder.count);
	verify( 45 == fibre_scheduler_next( 45) && fibre_self() == &yielder.fibre);
	verify( 50 == sleeper.time && 5 == yielder.count);
	verify( 50 == fibre_scheduler_next( 46) && fibre_self() == &yielder.fibre); // fibre completes
	//    ^^^^ note difference in idle time
	verify( 50 == sleeper.time && 5 == yielder.count);
	verify( 50 == fibre_scheduler_next( 47) && fibre_self() == NULL); // idle
	verify( 50 == sleeper.time && 5 == yielder.count);
	verify( 50+FIBRE_UNBOUNDED_SLEEP
		   == fibre_scheduler_next(50) && fibre_self() == &sleeper.fibre);
	verify( 50 == sleeper.time && 5 == yielder.count);

	fibre_run(&handler.evtq.fibre);
	verify(60+FIBRE_UNBOUNDED_SLEEP ==
		     fibre_scheduler_next(60)); // handler runs to wait
	assert(0 == handler.last_event.id);
	event_descriptor_t *pdesc;
	pdesc = fibre_eventq_claim(&handler.evtq);
	pdesc->id = 1;
	fibre_eventq_send(&handler.evtq, pdesc);
	verify(70+FIBRE_UNBOUNDED_SLEEP ==
		     fibre_scheduler_next(70)); // handler runs to wait
	assert(1 == handler.last_event.id);
}

/*
 * A test that runs fibres from varying positions within the timerq.
 */
static void sleep_test()
{
	static sleep_fibre_t sleeper[3] = {
		{
			.max_time = 50,
			.step = 10,
			.fibre = FIBRE_VAR_INIT(sleep_fibre)
		},
		{
			.max_time = 50,
			.step = 10,
			.fibre = FIBRE_VAR_INIT(sleep_fibre)
		},
		{
			.time = 5,
			.max_time = 50,
			.step = 10,
			.fibre = FIBRE_VAR_INIT(sleep_fibre)
		}
	};

	/* All three fibres are launched and executed until the system becomes idle */
	fibre_run(&sleeper[0].fibre);
	fibre_run(&sleeper[1].fibre);
	fibre_run(&sleeper[2].fibre);
	verify(  0 == fibre_scheduler_next(  0) && fibre_self() == &sleeper[0].fibre);
	verify(  0 == fibre_scheduler_next(  0) && fibre_self() == &sleeper[1].fibre);
	verify( 10 == fibre_scheduler_next(  0) && fibre_self() == &sleeper[2].fibre);
	verify( 10 == fibre_scheduler_next(  0) && fibre_self() == NULL);

	/* Run sleeper[0] and schedule until idle */
	fibre_run(&sleeper[0].fibre);
	verify( 10 == fibre_scheduler_next(  0) && fibre_self() == &sleeper[0].fibre);
	verify( 10 == fibre_scheduler_next(  0) && fibre_self() == NULL);

	/* Run sleeper[1] and schedule until idle */
	fibre_run(&sleeper[1].fibre);
	verify( 10 == fibre_scheduler_next(  0) && fibre_self() == &sleeper[1].fibre);
	verify( 10 == fibre_scheduler_next(  0) && fibre_self() == NULL);

	/* Run sleeper[2] and schedule until idle */
	fibre_run(&sleeper[2].fibre);
	verify( 10 == fibre_scheduler_next(  0) && fibre_self() == &sleeper[2].fibre);
	verify( 10 == fibre_scheduler_next(  0) && fibre_self() == NULL);

	/* Allow time to pass causing #0 and #1 to be runnable. Ensure #0 runs first since
	 * it was starting waiting first.
	 */
	verify( 10 == fibre_scheduler_next( 10) && fibre_self() == &sleeper[0].fibre);
	verify( 15 == fibre_scheduler_next( 10) && fibre_self() == &sleeper[1].fibre);
	verify( 15 == fibre_scheduler_next( 10) && fibre_self() == NULL);

	/* Allow time to pass causing #2 to be runnable */
	verify( 20 == fibre_scheduler_next( 15) && fibre_self() == &sleeper[2].fibre);
	verify( 20 == fibre_scheduler_next( 15) && fibre_self() == NULL);

	/* Make the fibres runnable in reverse order and allow time to pass, verify
	 * that the fibre_run() imposes the execution order.
	 */
	fibre_run(&sleeper[2].fibre);
	fibre_run(&sleeper[1].fibre);
	fibre_run(&sleeper[0].fibre);
	verify( 39 == fibre_scheduler_next( 39) && fibre_self() == &sleeper[2].fibre);
	verify( 39 == fibre_scheduler_next( 39) && fibre_self() == &sleeper[1].fibre);
	verify( 40 == fibre_scheduler_next( 39) && fibre_self() == &sleeper[0].fibre);
	verify( 40 == fibre_scheduler_next( 39) && fibre_self() == NULL);

	/* Allow sufficient time to pass for the fibres to complete. Here #1 will run
	 * before #0 since it joined the timer queue first.
	 */
	verify( 60 == fibre_scheduler_next( 60) && fibre_self() == &sleeper[1].fibre);
	verify( 60 == fibre_scheduler_next( 60) && fibre_self() == &sleeper[0].fibre);
	verify( 60+FIBRE_UNBOUNDED_SLEEP
		   == fibre_scheduler_next( 60) && fibre_self() == &sleeper[2].fibre);
	verify( 60+FIBRE_UNBOUNDED_SLEEP
		   == fibre_scheduler_next( 60) && fibre_self() == NULL);
}

static void yield_test()
{
	static yield_fibre_t yielder[3] = {
		{
			.max_count = 10,
			.fibre = FIBRE_VAR_INIT(atomic_fibre) // <--
		},
		{
			.max_count = 15,
			.fibre = FIBRE_VAR_INIT(yield_fibre) // <--
		},
		{
			.max_count = 10,
			.fibre = FIBRE_VAR_INIT(exit_fibre) // <--
		}
	};

	/* Launch the fibres */
	fibre_run(&yielder[0].fibre);
	fibre_run(&yielder[1].fibre);
	fibre_run(&yielder[2].fibre);

	for (int i=0; i<10; i++) {
		/* no change to run count */
		verify(yielder[0].count == i);
		verify(yielder[1].count == i);
		verify(yielder[2].count == i);

		/* schedule yielder 0 */
		verify(i == fibre_scheduler_next(i) &&
		       fibre_self() == &yielder[0].fibre);
		verify(yielder[0].count == i+1);
		verify(yielder[1].count == i);
		verify(yielder[2].count == i);

		/* schedule yielder 1 */
		verify(i == fibre_scheduler_next(i) &&
		       fibre_self() == &yielder[1].fibre);
		verify(yielder[0].count == i+1);
		verify(yielder[1].count == i+1);
		verify(yielder[2].count == i);

		/* schedule yielder 2 */
		verify(i == fibre_scheduler_next(i) &&
		       fibre_self() == &yielder[2].fibre);
		verify(yielder[0].count == i+1);
		verify(yielder[1].count == i+1);
		verify(yielder[2].count == i+1);
	}

	verify(10 == fibre_scheduler_next(10) &&
	       fibre_self() == &yielder[0].fibre); // exits
	verify(10 == fibre_scheduler_next(10) &&
	       fibre_self() == &yielder[1].fibre);
	verify(10 == fibre_scheduler_next(10) &&
	       fibre_self() == &yielder[2].fibre); // exits

	for (int i=11; i<15; i++) {
		/* no change to run count */
		verify(yielder[0].count == 10);
		verify(yielder[1].count ==  i);
		verify(yielder[2].count == 10);

		verify(i == fibre_scheduler_next(i) &&
		       fibre_self() == &yielder[1].fibre);
		verify(yielder[1].count ==  i+1);
	}

	verify(15+FIBRE_UNBOUNDED_SLEEP == fibre_scheduler_next(15) &&
	       fibre_self() == &yielder[1].fibre); // exits
	verify(15+FIBRE_UNBOUNDED_SLEEP == fibre_scheduler_next(15) &&
	       fibre_self() == NULL);
}

int main()
{
	basic_test();
	sleep_test();
	yield_test();

	return 0;
}
