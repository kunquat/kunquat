

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


#ifndef K_DSP_TYPE_H
#define K_DSP_TYPE_H


#include <stdint.h>

#include <devices/DSP.h>


/**
 * This is the type of a DSP implementation constructor.
 *
 * \return   The new DSP if successful, or \c NULL if memory allocation
 *           failed.
 */
typedef Device_impl* DSP_cons(DSP* dsp);


/**
 * This is the type of a DSP property function.
 *
 * A DSP property function is used to retrieve information about the internal
 * operation of the DSP. The property is returned as JSON data.
 *
 * \param dsp             The DSP -- must not be \c NULL.
 * \param property_type   The property to resolve -- must not be \c NULL.
 *
 * \return   The property for the DSP if one exists, otherwise \c NULL.
 */
typedef const char* DSP_property(DSP* dsp, const char* property_type);


typedef struct DSP_type DSP_type;


/**
 * Find a DSP constructor.
 *
 * \param type   The DSP type -- must not be \c NULL.
 *
 * \return   The constructor if \a type is supported, otherwise \c NULL.
 */
DSP_cons* DSP_type_find_cons(const char* type);


/**
 * Find a DSP property function.
 *
 * \param type   The DSP type -- must be a valid type.
 *
 * \return   The property function if one exists, otherwise \c NULL.
 */
DSP_property* DSP_type_find_property(const char* type);


#endif // K_DSP_TYPE_H


