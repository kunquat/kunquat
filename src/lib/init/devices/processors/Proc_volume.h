

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


#ifndef K_PROC_VOLUME_H
#define K_PROC_VOLUME_H


#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>

#include <stdlib.h>


typedef struct Proc_volume
{
    Device_impl parent;
    double scale;
} Proc_volume;


/**
 * Create a new volume processor.
 *
 * \return   The new volume processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_volume(void);


/**
 * Return volume Processor property information.
 *
 * \param proc            The volume Processor -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The volume Processor property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
const char* Proc_volume_property(const Processor* proc, const char* property_type);


#endif // K_PROC_VOLUME_H


