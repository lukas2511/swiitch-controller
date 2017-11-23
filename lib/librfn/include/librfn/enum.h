/*
 * enum.h
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2012-2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef RF_ENUM_H_
#define RF_ENUM_H_

/*!
 * \defgroup librfn_enum Enumerations
 *
 * \brief Convert enumerations to strings and strings to enumerations
 *
 * @{
 */

/*!
 * \brief Lookup table used for the conversions.
 *
 * This is normally initialized using a user supplied macro that takes
 * advantage of both the token concatenation operator (##) and the stringize
 * opeator (#). For example the following will produce a table to link the
 * enumerated value NAMESPACE_PREFIX_ALPHA to the string "ALPHA" (and so on).
 *
 * \code
 * #define E(x) { NAMESPACE_PREFIX_ ## x, #x }
 * rf_enumtable_t namespace_prefix_lookup[] = RF_ENUMTABLE_INITIALIZER(
 *	E(ALPHA),
 *	E(BRAVO),
 *	E(CHARLIE)
 * );
 * #undef E
 * \endcode
 */
typedef struct {
	const char *s;
	int e;
} rf_enumtable_t;

/*!
 * Initialize a enum lookup table.
 *
 * Using this macro optional but guarantees that the initializer is
 * correctly terminated.
 */
#define RF_ENUMTABLE_INITIALIZER(...) { __VA_ARGS__, RF_ENUMTABLE_TERMINATOR }

#define RF_ENUM_OUT_OF_RANGE -19830927
#define RF_ENUMTABLE_TERMINATOR { NULL, 0 }


const char *rf_enum2string(rf_enumtable_t *t, int e);
int rf_string2enum(rf_enumtable_t *t, const char *s);

/*! @} */
#endif // RF_ENUM_H_
