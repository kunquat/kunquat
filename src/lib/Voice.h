

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_VOICE_H
#define K_VOICE_H


#include <stdbool.h>
#include <stdint.h>

#include <Generator.h>
#include <player/Channel_gen_state.h>
#include <player/Device_states.h>
#include <Random.h>
#include <Voice_params.h>
#include <Voice_state.h>


typedef enum
{
    VOICE_PRIO_INACTIVE = 0,
    VOICE_PRIO_BG,
    VOICE_PRIO_FG,
    VOICE_PRIO_NEW
} Voice_prio;


/**
 * This contains all the playback state information of a single note of a
 * Generator being played.
 */
typedef struct Voice
{
    uint64_t id;           ///< An identification number for this initialisation.
    Voice_prio prio;       ///< Current priority of the Voice.
    Generator* gen;        ///< The Generator.
    size_t state_size;     ///< The amount bytes allocated for the Voice state.
    Voice_state* state;    ///< The current playback state.
    Random* rand_p;        ///< Parameter random source.
    Random* rand_s;        ///< Signal random source.
} Voice;


/**
 * Creates a new Voice.
 *
 * \param state_size   The amount of bytes to reserve for Voice states.
 *
 * \return   The new Voice if successful, or \c NULL if memory allocation
 *           failed.
 */
Voice* new_Voice(void);


/**
 * Reserves space for the Voice state.
 *
 * \param voice        The Voice -- must not be \c NULL.
 * \param state_size   The amount of bytes to reserve for the Voice state.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Voice_reserve_state_space(Voice* voice, size_t state_size);


/**
 * Compares priorities of two Voices.
 *
 * \param v1   The first Voice -- must not be \c NULL.
 * \param v2   The second Voice -- must not be \c NULL.
 *
 * \return   An integer less than, equal to or greater than zero if \a v1 is
 *           found, respectively, to be lower than, equal to or greater than
 *           \a v2 in priority.
 */
int Voice_cmp(Voice* v1, Voice* v2);


/**
 * Retrieves the Voice identification.
 *
 * The user should store the ID after retrieving the Voice from the Voice
 * pool.
 *
 * \param voice   The Voice -- must not be \c NULL.
 *
 * \return   The ID.
 */
uint64_t Voice_id(Voice* voice);


/**
 * Initialises the Voice for mixing.
 *
 * \param voice     The Voice -- must not be \c NULL.
 * \param gen       The Generator used -- must not be \c NULL.
 * \param params    The Voice parameters -- must not be \c NULL.
 * \param cgstate   The Channel-specific Generator state -- must not be
 *                  \c NULL.
 * \param seed      The random seed.
 * \param freq      The mixing frequency -- must be > \c 0.
 * \param tempo     The current tempo -- must be > \c 0.
 */
void Voice_init(Voice* voice,
                Generator* gen,
                Voice_params* params,
                Channel_gen_state* cgstate,
                uint64_t seed,
                uint32_t freq,
                double tempo);


/**
 * Resets the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 */
void Voice_reset(Voice* voice);


/**
 * Prepares the Voice for a new mixing cycle.
 *
 * \param voice   The Voice -- must not be \c NULL.
 */
void Voice_prepare(Voice* voice);


/**
 * Mixes the Voice.
 *
 * \param voice    The Voice -- must not be \c NULL.
 * \param states   The Device states -- must not be \c NULL.
 * \param amount   The number of frames to be mixed.
 * \param offset   The buffer offset.
 * \param freq     The mixing frequency -- must be > \c 0.
 * \param tempo    The current tempo -- must be > \c 0.
 */
void Voice_mix(
        Voice* voice,
        Device_states* states,
        uint32_t amount,
        uint32_t offset,
        uint32_t freq,
        double tempo);


/**
 * Returns the actual current force of the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL and must be active.
 *
 * \return   The actual force.
 */
double Voice_get_actual_force(Voice* voice);


/**
 * Destroys an existing Voice.
 *
 * \param voice   The Voice, or \c NULL.
 */
void del_Voice(Voice* voice);


#endif // K_VOICE_H


