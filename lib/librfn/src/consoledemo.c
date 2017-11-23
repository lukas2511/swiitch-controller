/*
 * consoledemo.c
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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "librfn.h"

/*
 * console_add is not protothreaded. It can simply return PT_EXITED.
 */
static pt_state_t console_add(console_t *c)
{
	if (c->argc != 3)
		fprintf(c->out, "Usage: add <x> <y>\n");
	else
		fprintf(c->out, "%ld\n", strtol(c->argv[1], NULL, 0) +
					     strtol(c->argv[2], NULL, 0));

	return PT_EXITED;
}
static const console_cmd_t cmd_add =
    CONSOLE_CMD_VAR_INIT("add", console_add);


/*
 * console_udelay is a busy-wait (scheduler cannot go idle) that yields to other
 * protothreads of fibres.
 */
static pt_state_t console_udelay(console_t *c)
{
	PT_BEGIN(&c->pt);

	if (c->argc != 2) {
		fprintf(c->out, "Usage: udelay <time>\n");
		PT_EXIT();
	}

	/* parse arguments to local variables and copy to scratch when argument
	 * parsing is complete.
	 */
	uint32_t delay = strtol(c->argv[1], NULL, 0);
	c->scratch.u32[0] = time_now();
	c->scratch.u32[1] = c->scratch.u32[0] + delay;

	while (cyclecmp32(time_now(), c->scratch.u32[1]) < 0)
		PT_YIELD();

	fprintf(c->out, "udelay completed after %uus\n",
		time_now() - c->scratch.u32[0]);

	PT_END();
}
static const console_cmd_t cmd_udelay =
    CONSOLE_CMD_VAR_INIT("udelay", console_udelay);

/*
 * console_usleep is a fully fledged sleeping wait based upon the librfn fibre
 * scheduler.
 */
static pt_state_t console_usleep(console_t *c)
{
	PT_BEGIN(&c->pt);

	if (c->argc != 2) {
		fprintf(c->out, "Usage: usleep <time>\n");
		PT_EXIT();
	}

	/* parse arguments to local variables and copy to scratch when argument
	 * parsing is complete.
	 */
	uint32_t delay = strtol(c->argv[1], NULL, 0);
	c->scratch.u32[0] = time_now();
	c->scratch.u32[1] = c->scratch.u32[0] + delay;

	PT_WAIT_UNTIL(fibre_timeout(c->scratch.u32[1]));

	fprintf(c->out, "usleep completed after %uus\n",
		time_now() - c->scratch.u32[0]);

	PT_END();
}
static const console_cmd_t cmd_usleep =
    CONSOLE_CMD_VAR_INIT("usleep", console_usleep);

int main(int argc, char *argv[])
{
	console_t console;

	console_init(&console, stdout);
	console_register(&cmd_usleep);
	console_register(&cmd_add);
	console_register(&cmd_udelay);

	fibre_scheduler_main_loop();

	return 0;
}
