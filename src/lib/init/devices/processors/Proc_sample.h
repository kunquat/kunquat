

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


#ifndef K_PROC_SAMPLE_H
#define K_PROC_SAMPLE_H


#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>

#include <stdlib.h>


#define SAMPLES_MAX (512)

#define SAMPLE_SOURCES_MAX (16)
#define SAMPLE_EXPRESSIONS_MAX (16)
#define SAMPLE_RANDOMS_MAX (8)


typedef struct Proc_sample
{
    Device_impl parent;
} Proc_sample;


/**
 * Create a new Sample processor.
 *
 * \return   The new Sample processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_sample(void);


/**
 * Return Sample Processor property information.
 *
 * \param proc            The Sample Processor -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The Sample Processor property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
const char* Proc_sample_property(const Processor* proc, const char* property_type);


#endif // K_PROC_SAMPLE_H


