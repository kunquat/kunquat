

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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
#include <Voice_params.h>

#include <Voice_state_sine.h>
#include <Voice_state_pcm.h>
#include <Voice_state_triangle.h>
#include <Voice_state_square.h>
#include <Voice_state_square303.h>
#include <Voice_state_sawtooth.h>
#include <Voice_state_noise.h>


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
    uint16_t pool_index;   ///< Storage position in the Voice pool.
    uint64_t id;           ///< An identification number for this initialisation.
    Voice_prio prio;       ///< Current priority of the Voice.
    bool was_fg;
    uint32_t fg_mixed;     ///< Number of frames mixed in the foreground (this mixing cycle).
    Generator* gen;        ///< The Generator.
    /// The current playback state.
    union
    {
        Voice_state generic;
        Voice_state_sine sine;
        Voice_state_pcm pcm;
        Voice_state_triangle triangle;
        Voice_state_square square;
        Voice_state_square303 square303;
        Voice_state_sawtooth sawtooth;
        Voice_state_noise noise;
    } state;
    uint16_t dyn_state_size;
    void* dyn_state;
} Voice;


/**
 * Creates a new Voice.
 *
 * \param dyn_state_size   Number of bytes to reserve for the Voice state.
 *
 * \return   The new Voice if successful, or \c NULL if memory allocation
 *           failed.
 */
Voice* new_Voice(uint16_t dyn_state_size);


/**
 * Reserves extra memory for the Voice state.
 *
 * \param voice            The Voice -- must not be \c NULL.
 * \param dyn_state_size   Number of bytes to reserve.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Voice_reserve_dyn_state_space(Voice* voice, uint16_t dyn_state_size);


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
 * \param voice    The Voice -- must not be \c NULL.
 * \param gen      The Generator used -- must not be \c NULL.
 * \param params   The Voice parameters -- must not be \c NULL.
 * \param freq     The mixing frequency -- must be > \c 0.
 * \param tempo    The current tempo -- must be > \c 0.
 */
void Voice_init(Voice* voice,
                Generator* gen,
                Voice_params* params,
                uint32_t freq,
                double tempo);


/**
 * Resets the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 */
void Voice_reset(Voice* voice);


/**
 * Adds a new Event into the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 * \param event   The Event -- must not be \c NULL.
 * \param pos     The position of the Event.
 *
 * \return   \c true if successful, or \c false if the Event queue is full.
 */
//bool Voice_add_event(Voice* voice, Event* event, uint32_t pos);


/**
 * Mixes the Voice.
 *
 * \param voice    The Voice -- must not be \c NULL.
 * \param amount   The number of frames to be mixed.
 * \param offset   The buffer offset.
 * \param freq     The mixing frequency -- must be > \c 0.
 * \param tempo    The current tempo -- must be > \c 0.
 */
void Voice_mix(Voice* voice,
               uint32_t amount,
               uint32_t offset,
               uint32_t freq,
               double tempo);


/**
 * Destroys an existing Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 */
void del_Voice(Voice* voice);


#endif // K_VOICE_H


