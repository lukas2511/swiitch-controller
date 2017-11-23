/*
 * console.h
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

#ifndef RF_CONSOLE_H_
#define RF_CONSOLE_H_

#include <stdio.h>

#include "fibre.h"
#include "ringbuf.h"

/*!
 * \defgroup librfn_console Command console
 *
 * \brief Lightweight command interpreter implemented as a protothreads
 *
 * The use of protothreads means this command interpreter can be used effectively
 * on run-to-completion schedulers and comes with out of the box support for fibres.
 *
 * \note The use of the fibre scheduler is optional (although the fibre header file
 *       is needed to compile source without modification).
 *
 * No dynamic memory allocation is required by the console handling (although
 * currently it uses stdio.
 *
 * @{
 */

struct console;

/*!
 * \brief Console command descriptor.
 *
 * Tip: If you register many commands with the same function pointer you can
 * parameterize them using containerof().
 */
typedef struct {
	const char *name;
	pt_state_t (*fn)(struct console *c);
} console_cmd_t;

#define CONSOLE_CMD_VAR_INIT(n, f)                                             \
	{                                                                      \
		.name = (n),                                                   \
		.fn = (f)                                                      \
	}

#define SCRATCH_SIZE 80

/*!
 * \brief Console descriptor.
 *
 * This is a relatively large structure (164 bytes on a 32-bit machine) due
 * to the integrated scratch array.
 */
typedef struct console {
	fibre_t fibre;

	FILE *out;

	char ringbuf[16];
	ringbuf_t ring;

	/*!
	 * Scratch buffer used by commands to store state.
	 *
	 * \warning Arguments passed contain pointers to these scratch buffers.
	 *          Thus commands must parse their command line before storing state
	 *          in the scratch buffers.
	 */
	union {
		char buf[SCRATCH_SIZE];
		uint8_t u8[SCRATCH_SIZE];
		uint16_t u16[SCRATCH_SIZE/2];
		uint32_t u32[SCRATCH_SIZE/4];
		void *p[SCRATCH_SIZE/4];
	} scratch;
	char *bufp;

	int argc;
	char *argv[4];

	const console_cmd_t *cmd;
	pt_t pt;
} console_t;

/*!
 * \brief Initialized the console handler.
 *
 * \param c Pointer to console descriptor
 * \param f File pointer to be used for all console output
 */
void console_init(console_t *c, FILE *f);

/*!
 * \brief Platform dependant function that will be called during console_init().
 *
 * librfn provides example implementations of this function but, in some cases,
 * this function must be provided by the application.
 */
void console_hwinit(console_t *c);

/*!
 * \brief Make the console silent at boot.
 *
 * Causes the command interpreter to issue no prompt when it boots.
 */
static inline void console_silent(console_t *c) { c->argc = 1; }

/*!
 * \brief Register a new command.
 *
 * The following example shows a simple protothreaded command to list the command's
 * arguments:
 *
 * \code
 * static pt_state_t listargs(console_t *c)
 * {
 *     static int i;
 *     PT_BEGIN(&c->pt);
 *
 *     for (i=0; i<c->argc; i++) {
 *         fprintf(c->out, "%d: %s\n", i, c->argv[i]);
 *         PT_YIELD();
 *     }
 *
 *     PT_END();
 * }
 * static const console_cmd_t cmd_listagrs =
 *     CONSOLE_CMD_VAR_INIT("listargs", listargs);
 *
 * (void) console_register(cmd_listargs);
 * \endcode
 *
 * The use of protothreading is optional. The following command is functionally
 * equivalent although may cause a run-to-completion scheduler to run poorly if
 * fprintf() is slow (for example if it synchronously sends characters to a serial
 * port):
 *
 * \code
 * pt_state_t listargs(console_t *c)
 * {
 *     for (int i=0; i<c->argc; i++)
 *         fprintf(c->out, "%d: %s\n", i, c->argv[i]);
 *     return PT_EXITED;
 * }
 * \endcode
 */
int console_register(const console_cmd_t *cmd);

/*!
 * \brief Asynchronously send a character to the command processor.
 *
 * This function is safe to call from interrupt. It will insert a character into
 * the command processors ring buffer and will, optionally, schedule the console
 * fibre.
 */
void console_putchar(console_t *c, char d);

/*! \brief Proto-thread to inject a string into the command parser.
 *
 * When buffering is already happening somewhere else in the system then this
 * is a safe alternative to console_putchar() since it will yield rather than
 * dropping input during overflow conditions.
 */
pt_state_t console_eval(pt_t *pt, console_t *c, const char *cmd);

/*!  \brief Fetch a character from the command processors queue.
 *
 * This function is used internally by the command processor and may
 * also be used by console commands to read character input from the
 * user. It must not be called outside of console command handling
 * (because it will race with the command processor to handle the
 * character).
 *
 * It is safe to use PT_WAIT_UNTIL() to wait until a character is
 * received because the console fibre, which is used to execute console
 * commands, will be woken up for each new character received.
 *
 * \code
 * // ch need not be static unless the it is used after another wait.
 * int ch;
 * PT_WAIT_UNTIL((ch = console_getchar(c)) != -1);
 * \endcode
 *
 */
int console_getch(console_t *c);

/*!
 * \brief Console protothread entrypoint.
 *
 * This function can be used to integrate the command processor into simple
 * polling loops or to "alien" run-to-completion schedulers.
 *
 * The return code can be used to realize power saving modes. However a simple
 * polling loop without support for power saving can simply ignore it.
 *
 * \code
 * while (true) {
 *     poll_something();
 *     (void) console_run(console); // poll console
 *     poll_something_else();
 * }
 * \endcode
 */
pt_state_t console_run(console_t *c);

/*!
 * \brief Synchronous console function for use in threaded environments.
 *
 * This function can also be used to implement a command console that executes
 * from an interrupt handler. Such a command interpreter would be extremely robust
 * although potentially at the cost of poor interrupt latencies.
 *
 * To use this function console_hwinit() may have to altered to remove code that
 * asynchronously delivers characters.
 *
 * \code
 * while ((ch = getchar()) != -1)
 *     console_process(console, ch);
 * \endcode
 */
void console_process(console_t *c, char d);

/*!
 * \brief Request special features from the GPIO command.
 */
typedef enum {
	console_gpio_active_low = 0x01,
	console_gpio_default_on = 0x02,
	console_gpio_open_drain = 0x04,
	console_gpio_explore = 0x08
} console_gpio_flags_t;

/*!
 * \brief GPIO command descriptor.
 *
 * This is a normal command descriptor augmented with details of the
 * GPIO port and pin to be targetted together with various flags.
 */
typedef struct {
	console_cmd_t cmd;
	uintptr_t port;
	uint32_t pin;
	int flags;
} console_gpio_t;

/*!
 * \private
 */
pt_state_t console_gpio_do_cmd(console_t *c);

#define CONSOLE_GPIO_VAR_INIT(name, port_, pin_, flags_)                       \
	{                                                                      \
		.cmd = CONSOLE_CMD_VAR_INIT(name, console_gpio_do_cmd),        \
		.port = port_, .pin = pin_, .flags = flags_                    \
	}

/*!
 * \brief Register a new GPIO command.
 */
int console_gpio_register(const console_gpio_t *gpio);

/*! @} */
#endif // RF_CONSOLE_H_
