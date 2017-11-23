/*
 * fibredemo.c
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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "librfn.h"


typedef union {
	enum update_event_id {
		UPDATE_EVENT_TIME,
		UPDATE_EVENT_EXIT,
	} id;

	/* all the remaining event structures must commence with enum
	 * update_event_id to ensure we never clobber the event id.
	 */

	struct {
		enum update_event_id id;
		unsigned int minutes;
		unsigned int seconds;
	} time;

	struct {
		enum update_event_id id;
		int status;
	} exit;
} update_event_t;

int update_handler(fibre_t *fibre)
{
	fibre_eventq_t *evtq = containerof(fibre, fibre_eventq_t, fibre);

	PT_BEGIN_FIBRE(fibre);

	while (true) {
		update_event_t *evt;

		/* wait until we receive an event to act upon */
		PT_WAIT_UNTIL(NULL != (evt = fibre_eventq_receive(evtq)));

		/* react to the event */
		switch(evt->id) {
		case UPDATE_EVENT_TIME:
			printf("\r%02d:%02d", evt->time.minutes,
					evt->time.seconds);
			fflush(stdout);
			break;
		case UPDATE_EVENT_EXIT:
			printf("\n\nTime is up\n");
			exit(evt->exit.status);
			break;
		default:
			assert(0);
			break;
		}

		/* the eventq is zero-copy so we must let it know when we
		 * have finished with the event.
		 */
		fibre_eventq_release(evtq, evt);
	}

	PT_END();
}

update_event_t update_eventq[8];
fibre_eventq_t updater = FIBRE_EVENTQ_VAR_INIT(
		update_handler, update_eventq, sizeof(update_eventq),
		sizeof(update_eventq[0]));


typedef struct {
	uint32_t time;
	unsigned int minutes;
	unsigned int seconds;
	unsigned int exit_after_minutes;
	unsigned int exit_after_seconds;
	fibre_t fibre;
} stopwatch_fibre_t;

int stopwatch_fibre(fibre_t *fibre)
{
	stopwatch_fibre_t *clock = containerof(fibre, stopwatch_fibre_t, fibre);

	PT_BEGIN_FIBRE(fibre);

	/* initialize the duetime from the current time (making this the the
	 * time from which the stopwatch will measure time)
	 */
	clock->time = time_now();

	while (true) {
		update_event_t *evt;

		/* send the current time to the event handler */
		if (NULL != (evt = fibre_eventq_claim(&updater))) {
			evt->id = UPDATE_EVENT_TIME;
			evt->time.minutes = clock->minutes;
			evt->time.seconds = clock->seconds;
			(void) fibre_eventq_send(&updater, evt);
		}
		/* "error handling" usually comes from errors being logged as
		 * scheduler "taints"
		 */

		/* has the stopwatch expired? */
		if ((clock->exit_after_minutes || clock->exit_after_seconds) &&
		    clock->exit_after_minutes <= clock->minutes &&
		    clock->exit_after_seconds <= clock->seconds &&
		    NULL != (evt = fibre_eventq_claim(&updater))) {
			evt->id = UPDATE_EVENT_EXIT;
			evt->exit.status = 0;
			(void) fibre_eventq_send(&updater, evt);
		}

		/* increment the duetime by a second and wait for the duetime
		 * to expire.
		 */
		clock->time += 1000000;
		PT_WAIT_UNTIL(fibre_timeout(clock->time));

		/* update the recorded time */
		clock->seconds += 1;
		if (clock->seconds >= 60) {
			clock->seconds = 0;
			clock->minutes += 1;
		}
	}

	PT_END();
}

stopwatch_fibre_t stopwatch = {
		.fibre = FIBRE_VAR_INIT(stopwatch_fibre)
};

int main(int argc, char *argv[])
{
	switch(argc) {
	int n;
	case 0:
	case 1:
		// do nothing
		break;
	default:
		fprintf(stderr, "Too many arguments - "
				"all except '%s' will be ignored\n", argv[1]);
		/* no break */
	case 2:
		n = sscanf(argv[1], "%d:%d",
				&stopwatch.exit_after_minutes,
				&stopwatch.exit_after_seconds);
		switch (n) {
		case 2:
			// do nothing
			break;
		case 1:
			stopwatch.exit_after_seconds =
					stopwatch.exit_after_minutes;
			stopwatch.exit_after_minutes = 0;
			break;
		case 0:
			fprintf(stderr, "Cannot extract meaningful time "
					"from '%s'\n", argv[1]);
			break;
		}
		break;
	}

	fibre_run(&stopwatch.fibre);
	fibre_run(&updater.fibre);

	fibre_scheduler_main_loop();

	return 0;
}
