/*
 * mlog.h
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2015 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef RF_MLOG_H_
#define RF_MLOG_H_

#include <stdio.h>
#include <stdarg.h>

/*!
 * \defgroup librfn_mlog In-memory logging
 *
 * \brief Log text messages to a circular memory buffer
 *
 * A very fast text-centric circular log with deferred string formatting.
 * Deferred formatting permits extremely low overhead logging; Each message is
 * recorded using just four pointer sized writes. The log can be decoded either
 * on the target device or using a external debugger. 
 *
 * This comes at a cost. In particular it imposes some significant restrictions
 * on formatting compared to typical printf() influenced logging tools.
 *
 * @{
 */

/*!
 * \brief Log a message using a variable argument list.
 *
 * See mlog() for for further details.
 */
void vmlog(const char *fmt, va_list ap);

/*!
 * \brief Log a message.
 *
 * In order to log a message mlog() captures the format string and up to
 * three variadic arguments. It will not format the output during logging
 * and as a result has an extremely low runtime overhead.
 *
 * Some limitations arise as a result of deferring the string formatting.
 * In particular:
 *
 * 1. Both the format string and any other strings to be displayed must
 *    never be overwritten with a different value. In practice this typically
 *    means they must either be string literals or pointers to constant data.
 *
 * 2. It is not possible to format floating point or long long values on
 *    32-bit machines.
 *
 * 3. It is not possible to log 32-bit value on a 64-bit machine whose
 *    calling conventions do not pass the first three variadic arguments in
 *    registers.
 *
 * If you do not understand #3 above, don't worry too much! The problems
 * do not affect x86_64 systems.
 */
void mlog(const char *fmt, ...);

/*!
 * \brief Log a message using a variable argument list, if there is space to
 *        do so
 *
 * See mlog() for for further details.
 */
void vmlog_nice(const char *fmt, va_list ap);

/*!
 * \brief Log a message, if there is space to do so.
 *
 * See mlog() for for further details.
 */
void mlog_nice(const char *fmt, ...);

/*!
 * \brief Clear all data from the log.
 */
void mlog_clear(void);

/*!
 * \brief Format the log and write it to the supplied file pointer.
 */
void mlog_dump(FILE *f);

/*!  \brief Format the Nth line of the log.
 *
 * The string returned is dynamically allocated and should be freed using
 * free().
 */
char *mlog_get_line(int n);


/*! @} */
#endif // RF_MLOG_H_
