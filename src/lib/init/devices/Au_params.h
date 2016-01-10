

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


#ifndef K_AU_PARAMS_H
#define K_AU_PARAMS_H


#include <stdint.h>
#include <stdlib.h>


typedef struct Au_params
{
    uint32_t device_id;
} Au_params;


/**
 * Initialise the Audio unit parameters.
 *
 * \param aup         The Audio unit parameters -- must not be \c NULL.
 * \param device_id   The audio unit device ID -- must be > \c 0.
 *
 * \return   The parameter \a aup if successful, or \c NULL if memory
 *           allocation failed.
 */
Au_params* Au_params_init(Au_params* aup, uint32_t device_id);


/**
 * Deinitialise the Audio unit parameters.
 *
 * \param aup   The Audio unit parameters, or \c NULL.
 */
void Au_params_deinit(Au_params* aup);


#endif // K_AU_PARAMS_H


