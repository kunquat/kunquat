

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

#include <Event_queue.h>
#include <Generator.h>
#include <Channel_state.h>
#include <Voice_state_sine.h>
#include <Voice_state_pcm.h>
#include <Voice_state_triangle.h>
#include <Voice_state_square.h>
#include <Voice_state_square303.h>
#include <Voice_state_sawtooth.h>


typedef enum
{
    VOICE_PRIO_INACTIVE = 0,
    VOICE_PRIO_BG,
    VOICE_PRIO_FG,
    VOICE_PRIO_NEW
} Voice_prio;


/**
 * This contains all the playback state information of a single note being
 * played.
 */
typedef struct Voice
{
    uint16_t pool_index;   ///< Storage position in the Voice pool.
    uint64_t id;           ///< An identification number for this initialisation.
    Voice_prio prio;       ///< Current priority of the Voice.
    bool was_fg;
    uint32_t fg_mixed;     ///< Number of frames mixed in the foreground (this mixing cycle).
    Event_queue* events;   ///< Upcoming events.
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
    } state;
} Voice;


/**
 * Creates a new Voice.
 *
 * \param events   The maximum number of events per tick -- must be > \c 0.
 *
 * \return   The new Voice if successful, or \c NULL if memory allocation
 *           failed.
 */
Voice* new_Voice(uint8_t events);


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
 * \param voice          The Voice -- must not be \c NULL.
 * \param gen            The Generator used -- must not be \c NULL.
 * \param cur_ch_state   The current Channel state -- must not be \c NULL.
 * \param new_ch_state   The new (upcoming) Channel state -- must not be \c NULL.
 * \param freq           The mixing frequency -- must be > \c 0.
 * \param tempo          The current tempo -- must be > \c 0.
 */
void Voice_init(Voice* voice,
                Generator* gen,
                Channel_state* cur_ch_state,
                Channel_state* new_ch_state,
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
bool Voice_add_event(Voice* voice, Event* event, uint32_t pos);


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


