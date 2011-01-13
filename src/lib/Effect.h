

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EFFECT_H
#define K_EFFECT_H


#include <stdint.h>

#include <Connections.h>
#include <Device.h>
#include <DSP_table.h>
#include <File_base.h>


typedef struct Effect Effect;


/**
 * Creates a new Effect.
 *
 * \param buf_len    The length of a mixing buffer -- must be > \c 0.
 * \param mix_rate   The mixing rate -- must be > \c 0.
 *
 * \return   The new Effect if successful, or \c NULL if memory allocation
 *           failed.
 */
Effect* new_Effect(uint32_t buf_len,
                   uint32_t mix_rate);


/**
 * Parses an Effect header from a textual description.
 *
 * \param eff     The Effect -- must not be \c NULL.
 * \param str     The textual description.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Effect_parse_header(Effect* eff, char* str, Read_state* state);


/**
 * Gets a DSP of the Effect.
 *
 * \param eff     The Effect -- must not be \c NULL.
 * \param index   The index of the DSP -- must be >= \c 0 and
 *                < \c KQT_DSPS_MAX.
 *
 * \return   The DSP if one exists, otherwise \c NULL.
 */
DSP* Effect_get_dsp(Effect* eff, int index);


/**
 * Gets the DSP table of the Effect.
 *
 * \param eff   The Effect -- must not be \c NULL.
 *
 * \return   The DSP table.
 */
DSP_table* Effect_get_dsps(Effect* eff);


/**
 * Sets the Connections of the Effect.
 *
 * Previously set Connections will be removed if found.
 *
 * \param eff     The Effect -- must not be \c NULL.
 * \param graph   The Connections, or \c NULL.
 */
void Effect_set_connections(Effect* eff, Connections* graph);


/**
 * Destroys an existing Effect.
 *
 * \param eff   The Effect, or \c NULL.
 */
void del_Effect(Effect* eff);


#endif // K_EFFECT_H


