

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdint.h>

#include <Real.h>
#include <Reltime.h>


/**
 * Creates a JSON representation of a boolean value.
 *
 * \param dest    The destination string buffer -- must not be \c NULL.
 * \param size    The size of the string buffer including the terminating
 *                byte -- must be positive.
 * \param value   The value to be serialised.
 *
 * \return   The number of characters written to \a dest, not including the
 *           terminating byte.
 */
int serialise_bool(char* dest, int size, bool value);


/**
 * Creates a JSON representation of a integer value.
 *
 * \param dest    The destination string buffer -- must not be \c NULL.
 * \param size    The size of the string buffer including the terminating
 *                byte -- must be positive.
 * \param value   The value to be serialised.
 *
 * \return   The number of characters written to \a dest, not including the
 *           terminating byte.
 */
int serialise_int(char* dest, int size, int64_t value);


/**
 * Creates a JSON representation of a floating point value.
 *
 * \param dest    The destination string buffer -- must not be \c NULL.
 * \param size    The size of the string buffer including the terminating
 *                byte -- must be positive.
 * \param value   The value to be serialised -- must be finite.
 *
 * \return   The number of characters written to \a dest, not including the
 *           terminating byte.
 */
int serialise_float(char* dest, int size, double value);


/**
 * Creates a JSON representation of a Real value.
 *
 * \param dest    The destination string buffer -- must not be \c NULL.
 * \param size    The size of the string buffer including the terminating
 *                byte -- must be positive.
 * \param value   The value to be serialised -- must not be \c NULL.
 *
 * \return   The number of characters written to \a dest, not including the
 *           terminating byte.
 */
int serialise_Real(char* dest, int size, Real* value);


/**
 * Creates a JSON representation of a Timestamp value.
 *
 * \param dest    The destination string buffer -- must not be \c NULL.
 * \param size    The size of the string buffer including the terminating
 *                byte -- must be positive.
 * \param value   The value to be serialised -- must not be \c NULL.
 *
 * \return   The number of characters written to \a dest, not including the
 *           terminating byte.
 */
int serialise_Timestamp(char* dest, int size, Reltime* value);


