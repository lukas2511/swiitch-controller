/*
 * console.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <librfn/console.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <librfn/util.h>

static pt_state_t console_echo(console_t *c)
{
	for (int i=1; i<c->argc; i++)
		fprintf(c->out, " %s", c->argv[i]);
	fprintf(c->out, "\n");
	return PT_EXITED;
}
static const console_cmd_t cmd_echo =
    CONSOLE_CMD_VAR_INIT("echo", console_echo);

static pt_state_t console_help(console_t *c);
static const console_cmd_t cmd_help =
    CONSOLE_CMD_VAR_INIT("help", console_help);

static pt_state_t console_unknown(console_t *c)
{
	if (c->argv[0][0])
		fprintf(c->out, "Unknown/bad command\n");
	return PT_EXITED;
}
static const console_cmd_t cmd_unknown =
    CONSOLE_CMD_VAR_INIT(NULL, console_unknown);

static const console_cmd_t *cmd_table[32] = {
	&cmd_echo,
	&cmd_help,
	&cmd_unknown
};

static pt_state_t console_help(console_t *c)
{
	PT_BEGIN(&c->pt);

	fprintf(c->out, "Available commands:\n");
	PT_YIELD();

	static bool locked;
	PT_WAIT_UNTIL(!locked);
	locked = true;

	static const console_cmd_t **cmd;

	for (cmd=cmd_table; (*cmd)->name; cmd++) {
		fprintf(c->out, "  %s\n", (*cmd)->name);
		PT_YIELD();
	}

	locked = false;
	PT_END();
}

static void do_tokenize(console_t *c)
{
	c->argc = 1;
	c->argv[0] = c->scratch.buf;

	/* do the actual tokenization */
	size_t len = strlen(c->scratch.buf);
	char quote = '\0';
	for (size_t i = 1; i < len; i++) {
		if (isspace((int) c->scratch.buf[i]) && !quote) {
			c->scratch.buf[i] = '\0';
			continue;
		}

		if (c->scratch.buf[i] == quote) {
			quote = '\0';
			c->scratch.buf[i] = '\0';
			continue;
		}

		if (c->scratch.buf[i - 1] == '\0') {
			if (c->scratch.buf[i] == '\'' ||
			    c->scratch.buf[i] == '"') {
				quote = c->scratch.buf[i];
				c->scratch.buf[i] = '\0';
			} else {
				c->argv[c->argc] = c->scratch.buf + i;
				if (++c->argc >= (int) lengthof(c->argv))
					break;
			}
		}
	}

	/* ensure all arguments are NULL terminated so we can
	 * avoid error checking in the commands themselves
	 */
	for (size_t i = c->argc; i < lengthof(c->argv); i++)
		c->argv[i] = c->scratch.buf + len;
}

static void find_command(console_t *c)
{
	const console_cmd_t **cmd = cmd_table;
	for (cmd = cmd_table; (*cmd)->name; cmd++) {
		if (0 == strcmp(c->argv[0], (*cmd)->name))
			break;
	}
	c->cmd = *cmd;
}

static void do_prompt(console_t *c)
{
	/* get ready to read the command */
	memset(c->scratch.buf, 0, sizeof(c->scratch));
	c->bufp = c->scratch.buf;

	/* show the prompt */
	fprintf(c->out, "> ");
	fflush(c->out);
}

void console_init(console_t *c, FILE *f)
{
	memset(c, 0, sizeof(console_t));

	c->out = f;
	ringbuf_init(&c->ring, c->ringbuf, sizeof(c->ringbuf));

	console_hwinit(c);

#ifndef CONFIG_NO_FIBRE
	fibre_init(&c->fibre, (fibre_entrypoint_t *) console_run);
	fibre_run(&c->fibre);
#endif
}

int console_register(const console_cmd_t *cmd)
{
	size_t i, j;

	/* fail if the last command slot is already full */
	if (cmd_table[lengthof(cmd_table)-1])
		return -1;

	for (i = 0; i < lengthof(cmd_table); i++)
		if (NULL == cmd_table[i]->name ||
		    strcmp(cmd_table[i]->name, cmd->name) > 0)
			break;

	for (j = lengthof(cmd_table) - 1; j > i; j--)
		cmd_table[j] = cmd_table[j-1];

	cmd_table[i] = cmd;

	return 0;
}

int console_getch(console_t *c)
{
	return ringbuf_get(&c->ring);
}

void console_putchar(console_t *c, char d)
{
	/* this deliberately does not use ringbuf_putchar() because in most
	 * practical systems console_putchar() will be called from an ISR
	 * (which ringbuf_putchar() doesn't like)
	 */
	ringbuf_put(&c->ring, d);
#ifndef CONFIG_NO_FIBRE
	fibre_run_atomic(&c->fibre);
#endif
}

pt_state_t console_eval(pt_t *pt, console_t *c, const char *cmd)
{
	uint16_t *i = &c->scratch.u16[sizeof(c->scratch.u16)-1];

	PT_BEGIN(pt);

	for (*i=0; cmd[*i]; ) {
		if (ringbuf_put(&c->ring, cmd[*i])) {
			*i += 1;
		} else {
#ifndef CONFIG_NO_FIBRE
			fibre_run(&c->fibre);
#endif
			PT_YIELD();
		}
	}

#ifndef CONFIG_NO_FIBRE
	fibre_run(&c->fibre);
#endif
	PT_END();
}

/* This function must remain safe to cast to fibre_entrypoint_t in order to
 * inter-operate with the fibre scheduler. Currently pt_state_t is a enum (and
 * therefore is ABI compatible with int) and fibre is the first member of the
 * console_t structure.
 */
pt_state_t console_run(console_t *c)
{
	PT_BEGIN_FIBRE(&c->fibre);

	if (!c->argc)
		do_prompt(c);
	else
		c->bufp = c->scratch.buf;

	while (1) {
		int ch;
		PT_WAIT_UNTIL((ch = console_getch(c)) != -1);

		if (ch == '\n' || c->bufp >= &c->scratch.buf[79]) {
			do_tokenize(c);
			find_command(c);
			PT_SPAWN(&c->pt, c->cmd->fn(c));
			do_prompt(c);
		} else if (ch == '\b') {
			if (c->bufp > c->scratch.buf) {
				c->bufp--;
				fprintf(c->out, " \b");
			} else {
				fprintf(c->out, " ");
			}
			fflush(c->out);
		} else if (ch == 3) { /* Ctrl-C */
			fprintf(c->out, "\n");
			do_prompt(c);
		} else if (ch != '\n') {
			*c->bufp++ = ch;
		}
	}

	PT_END();
}

void console_process(console_t *c, char d)
{
	pt_state_t s;

	/* do *not* use console_putchar() we don't want the fibre to be run */
	ringbuf_put(&c->ring, d);

	do {
		s = console_run(c);
	} while (s == PT_YIELDED);
}

