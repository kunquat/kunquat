

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


#ifndef K_PROC_PULSE_H
#define K_PROC_PULSE_H


#include <devices/Device_impl.h>
#include <devices/Processor.h>


/**
 * Create a new Pulse Processor.
 *
 * \return   The new Pulse Processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_pulse(Processor* proc);


/**
 * Return Pulse Processor property information.
 *
 * \param proc            The Pulse Processor -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The Pulse Processor property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
const char* Proc_pulse_property(const Processor* proc, const char* property_type);


#endif // K_PROC_PULSE_H


