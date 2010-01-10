

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#ifndef K_GENERATOR_DEBUG_H
#define K_GENERATOR_DEBUG_H


#include <Generator.h>
#include <Voice_state.h>


typedef struct Generator_debug
{
    Generator parent;
} Generator_debug;


/**
 * Creates a new Debug Generator.
 *
 * \param ins_params   The Instrument parameters -- must not be \c NULL.
 *
 * \return   The new Debug Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator_debug* new_Generator_debug(Instrument_params* ins_params);


uint32_t Generator_debug_mix(Generator* gen,
                             Voice_state* state,
                             uint32_t nframes,
                             uint32_t offset,
                             uint32_t freq,
                             double tempo,
                             int buf_count,
                             kqt_frame** bufs);


/**
 * Destroys an existing Debug Generator.
 *
 * \param gen   The Debug Generator -- must not be \c NULL.
 */
void del_Generator_debug(Generator* gen);


#endif // K_GENERATOR_DEBUG_H


