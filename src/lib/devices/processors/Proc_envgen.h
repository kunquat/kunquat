

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_ENVGEN_H
#define K_PROC_ENVGEN_H


#include <stdlib.h>

#include <devices/Device_impl.h>
#include <devices/Processor.h>


/**
 * Create a new envelope generator Processor.
 *
 * \return   The new envelope generator Processor if successful, or \c NULL if
 *           memory allocation failed.
 */
Device_impl* new_Proc_envgen(Processor* proc);


/**
 * Return envelope generator Processor property information.
 *
 * \param proc            The envelope generator Processor -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 */
const char* Proc_envgen_property(const Processor* proc, const char* property_type);


#endif // K_PROC_ENVGEN_H


