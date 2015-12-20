

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_TYPE_H
#define K_PROC_TYPE_H


#include <devices/Device_impl.h>
#include <devices/Processor.h>

#include <stdlib.h>


#define PROC_TYPE_LENGTH_MAX 128


/**
 * This is the type of a Processor implementation constructor.
 *
 * \return   The new Processor if successful, or \c NULL if memory allocation
 *           failed.
 */
typedef Device_impl* Proc_cons(Processor* proc);


/**
 * This is the type of a Processor property function.
 *
 * A Processor property function is used to retrieve information about the
 * internal operation of the Processor. The property is returned as JSON data.
 * Most importantly, the function may return the amount of bytes the Processor
 * needs for Voice states to be available (property type "voice_state_size").
 *
 * \param proc            The Processor -- must not be \c NULL.
 * \param property_type   The property to resolve -- must not be \c NULL.
 *
 * \return   The property for the Processor if one exists, otherwise \c NULL.
 */
typedef const char* Proc_property(const Processor* proc, const char* property_type);


typedef struct Proc_type Proc_type;


/**
 * Find a Processor constructor.
 *
 * \param type   The Processor type -- must not be \c NULL.
 *
 * \return   The constructor if \a type is supported, otherwise \c NULL.
 */
Proc_cons* Proc_type_find_cons(const char* type);


/**
 * Find a Processor property function.
 *
 * \param type   The Processor type -- must be a valid type.
 *
 * \return   The property function if one exists, otherwise \c NULL.
 */
Proc_property* Proc_type_find_property(const char* type);


#endif // K_PROC_TYPE_H


