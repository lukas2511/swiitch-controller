/*
 * regdump.h
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

#ifndef RF_REGDUMP_H_
#define RF_REGDUMP_H_

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>

/*!
 * \defgroup librfn_regdump Register dump
 *
 * \brief Symbolic register dump routines
 *
 * @{
 */

#ifdef CONFIG_REGDUMP64
typedef uint64_t uintreg_t;
#define PRIxreg PRIx64
#else
typedef uint32_t uintreg_t;
#define PRIxreg PRIx32
#endif

/*!
 * \brief Register description structure
 *
 * The following example shows a simple UART RX register combining the
 * received character with some flags:
 *
 * \code
 * static const regdump_desc_t uart_rx_desc = {
 *     { "UART_RX" },
 *     { "CH", 0x00ff },
 *     { "FRM", 0x2000 },
 *     { "OVF", 0x4000 },
 *     { "BRK", 0x8000 }
 * };
 *
 * regdump(UART_RX(UART0), &uart_rx_desc);
 * \endcode
 */
typedef struct regdump_desc {
	const char *name;
	uintreg_t mask;
} regdump_desc_t;

/*!
 * \brief Static initializer for re-entrancy state variable.
 */
#define REGDUMP_STATE_VAR_INIT 0

/*!
 * \brief Dump single line of register output to file.
 *
 * This function works a line at a time and returns to the caller
 * between each line. It is only useful with run-to-completion
 * schedulers as a means to boost responsiveness.
 *
 * \returns Zero if the register dump is complete, non-zero otherwise.
 */
int fregdump_single(FILE *f, uintreg_t reg, const regdump_desc_t *desc,
		    int *state);

/*!
 * \brief Dump the current register status to file.
 */
void fregdump(FILE *f, uintreg_t reg, const regdump_desc_t *desc);

/*!
 * \brief Dump single line of register output to stdout.
 *
 * \see fregdump_single()
 * \returns Zero if the register dump is complete, non-zero otherwise.
 */
int regdump_single(uintreg_t reg, const regdump_desc_t *desc, int *state);

/*!
 * \brief Dump the current register status to stdout.
 */
void regdump(uintreg_t reg, const regdump_desc_t *desc);

/*! @} */
#endif // RF_REGDUMP_H_
