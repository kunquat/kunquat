

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2010-2015
 *          Ossi Saresoja, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_NOISE_H
#define K_PROC_NOISE_H


#include <devices/Device_impl.h>
#include <devices/Processor.h>

#include <stdlib.h>


typedef struct Proc_noise
{
    Device_impl parent;
} Proc_noise;


/**
 * Create a new Noise Processor.
 *
 * \return   The new Noise Processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_noise(Processor* proc);


/**
 * Return Noise Processor property information.
 *
 * \param proc            The Noise Processor -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The Noise Processor property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
const char* Proc_noise_property(const Processor* proc, const char* property_type);


#endif // K_PROC_NOISE_H


