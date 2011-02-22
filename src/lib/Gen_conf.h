

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


#ifndef K_GEN_CONF_H
#define K_GEN_CONF_H


#include <stdbool.h>

#include <Device.h>
#include <Device_params.h>
#include <File_base.h>
#include <pitch_t.h>
#include <Random.h>


/**
 * This contains all the Generator information that originates from one or
 * more files.
 */
typedef struct Gen_conf
{
    Device_params* params;
    bool enabled;
    double volume_dB;
    double volume;
    bool pitch_lock_enabled;
    double pitch_lock_cents;
    pitch_t pitch_lock_freq;
    Random* random;
} Gen_conf;


/**
 * Creates a new Generator configuration.
 *
 * \return   The new Generator configuration if successful, or \c NULL if
 *           memory allocation failed.
 */
Gen_conf* new_Gen_conf(void);


/**
 * Parses a key related to the Generator configuration.
 *
 * \param conf     The Generator configuration -- must not be \c NULL.
 * \param key      The key -- must be a valid subkey.
 * \param data     The data -- must not be \c NULL if it has a non-zero
 *                 length.
 * \param length   The length of the data -- must be >= \c 0.
 * \param device   The Generator Device if it exists, otherwise \c NULL.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a state will not be
 *           modified if memory allocation failed.
 */
bool Gen_conf_parse(Gen_conf* conf,
                    const char* key,
                    void* data,
                    long length,
                    Device* device,
                    Read_state* state);


/**
 * Destroys an existing Generator configuration.
 *
 * \param conf   The Generator configuration, or \c NULL.
 */
void del_Gen_conf(Gen_conf* conf);


#endif // K_GEN_CONF_H


