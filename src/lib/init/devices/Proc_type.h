

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PROC_TYPE_H
#define KQT_PROC_TYPE_H


#include <decl.h>

#include <stdlib.h>


typedef enum
{
#define PROC_TYPE(name) Proc_type_##name,
#include <init/devices/Proc_types.h>
    Proc_type_COUNT
} Proc_type;


#define PROC_TYPE_NAME_LENGTH_MAX 128


/**
 * The type of a Processor implementation constructor.
 *
 * \return   The new Processor if successful, or \c NULL if memory allocation
 *           failed.
 */
typedef Device_impl* Proc_cons(void);


/**
 * Get the Processor type from a type name.
 *
 * \param type_name   The name of the Processor type -- must not be \c NULL.
 *
 * \return   The Processor type if \a type_name is supported, otherwise
 *           \c Proc_type_COUNT.
 */
Proc_type Proc_type_get_from_string(const char* type_name);


/**
 * Get the constructor of a Processor type.
 *
 * \param type   The Processor type -- must be valid.
 *
 * \return   The Processor constructor.
 */
Proc_cons* Proc_type_get_cons(Proc_type type);


#endif // KQT_PROC_TYPE_H


