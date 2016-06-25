

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
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


#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>

#include <stdlib.h>


#define PROC_TYPE_LENGTH_MAX 128


/**
 * This is the type of a Processor implementation constructor.
 *
 * \return   The new Processor if successful, or \c NULL if memory allocation
 *           failed.
 */
typedef Device_impl* Proc_cons(void);


/**
 * Find a Processor constructor.
 *
 * \param type   The Processor type -- must not be \c NULL.
 *
 * \return   The constructor if \a type is supported, otherwise \c NULL.
 */
Proc_cons* Proc_type_find_cons(const char* type);


#endif // KQT_PROC_TYPE_H


