

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_VOICE_H
#define K_VOICE_H


#include <stdint.h>

#include <Event_queue.h>
#include <Generator.h>
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
 * \param voice   The Voice -- must not be \c NULL.
 * \param gen     The Generator used -- must not be \c NULL.
 */
void Voice_init(Voice* voice, Generator* gen);


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
 */
void Voice_mix(Voice* voice,
        uint32_t amount,
        uint32_t offset,
        uint32_t freq);


/**
 * Destroys an existing Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 */
void del_Voice(Voice* voice);


#endif // K_VOICE_H


