

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GEN_TYPE_H
#define K_GEN_TYPE_H


#include <stdint.h>

#include <devices/Device_impl.h>
#include <Generator.h>


/**
 * This is the type of a Generator implementation constructor.
 *
 * \return   The new Generator if successful, or \c NULL if memory allocation
 *           failed.
 */
typedef Device_impl* Generator_cons(Generator* gen);


/**
 * This is the type of a Generator property function.
 *
 * A Generator property function is used to retrieve information about the
 * internal operation of the Generator. The property is returned as JSON data.
 * Most importantly, the function may return the amount of bytes the Generator
 * needs for Voice states to be available (property type "voice_state_size").
 *
 * \param gen             The Generator -- must not be \c NULL.
 * \param property_type   The property to resolve -- must not be \c NULL.
 *
 * \return   The property for the Generator if one exists, otherwise \c NULL.
 */
typedef char* Generator_property(Generator* gen, const char* property_type);


typedef struct Gen_type Gen_type;


/**
 * Finds a Generator constructor.
 *
 * \param type   The Generator type -- must not be \c NULL.
 *
 * \return   The constructor if \a type is supported, otherwise \c NULL.
 */
Generator_cons* Gen_type_find_cons(const char* type);


/**
 * Finds a Generator property function.
 *
 * \param type   The Generator type -- must be a valid type.
 *
 * \return   The property function if one exists, otherwise \c NULL.
 */
Generator_property* Gen_type_find_property(const char* type);


#endif // K_GEN_TYPE_H


