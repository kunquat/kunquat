

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DSP_CONF_H
#define K_DSP_CONF_H


#include <stdbool.h>

#include <Device.h>
#include <Device_params.h>
#include <File_base.h>


/**
 * This contains all the DSP information that originates from one or more
 * files.
 */
typedef struct DSP_conf
{
    Device_params* params;
} DSP_conf;


/**
 * Creates a new DSP configuration.
 *
 * \return   The new DSP configuration if successful, or \c NULL if memory
 *           allocation failed.
 */
DSP_conf* new_DSP_conf(void);


/**
 * Parses a key related to the DSP configuration.
 *
 * \param conf     The DSP configuration -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey.
 * \param data     The data -- must not be \c NULL if it has a non-zero
 *                 length.
 * \param length   The length of the data -- must be >= \c 0.
 * \param device   The DSP Device if one exists, otherwise \c NULL.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a state will not be
 *           modified if memory allocation failed.
 */
bool DSP_conf_parse(DSP_conf* conf,
                    const char* key,
                    void* data,
                    long length,
                    Device* device,
                    Read_state* state);


/**
 * Destroys an existing DSP configuration.
 *
 * \param conf   The DSP configuration, or \c NULL.
 */
void del_DSP_conf(DSP_conf* conf);


#endif // K_DSP_CONF_H


