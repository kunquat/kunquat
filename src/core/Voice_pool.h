

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


#ifndef K_VOICE_POOL_H
#define K_VOICE_POOL_H


#include <stdint.h>
#include <stdbool.h>

#include <Voice.h>


/**
 * Voice pool manages the allocation of Voices.
 */
typedef struct Voice_pool
{
    uint16_t size;
    uint8_t events;
    Voice** voices;
} Voice_pool;


/**
 * Creates a new Voice pool.
 *
 * \param size     The number of Voices in the Voice pool -- must be > \c 0.
 * \param events   The maximum number of events per Voice per tick -- must be
 *                 > \c 0.
 *
 * \return   The new Voice pool if successful, or \c NULL if memory allocation
 *           failed.
 */
Voice_pool* new_Voice_pool(uint16_t size, uint8_t events);


/**
 * Changes the amount of Voices in the Voice pool.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 * \param size   The new size -- must be > \c 0.
 *
 * \return   \c true if resizing succeeded, or \c false if memory allocation
 *           failed. Note that decreasing the number of Voices may still have
 *           occurred even if the operation fails.
 */
bool Voice_pool_resize(Voice_pool* pool, uint16_t size);


/**
 * Gets the amount of Voices in the Voice pool.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 *
 * \return   The number of Voices.
 */
uint16_t Voice_pool_get_size(Voice_pool* pool);


/**
 * Gets a Voice from the Voice pool.
 *
 * In case all the Voices are in use, the Voice considered least important is
 * reinitialised and returned.
 *
 * If the caller gives an existing Voice as a parameter, no new Voice will be
 * returned. Instead, the Voice pool will check whether this Voice has the
 * same ID as the caller provides (if yes, the caller is still allowed to use
 * this Voice and the \a voice parameter itself will be returned; otherwise
 * \c NULL).
 *
 * \param pool      The Voice pool -- must not be \c NULL.
 * \param voice     An existing Voice. If \c NULL is returned, the caller must
 *                  not access this Voice.
 * \param id        An identification number for an existing Voice that needs
 *                  to be matched.
 *
 * \return   The Voice reserved for use, or \c NULL if \a voice is no longer
 *           under the control of the caller.
 */
Voice* Voice_pool_get_voice(Voice_pool* pool,
        Voice* voice,
        uint64_t id);


/**
 * Mixes the Voice pool.
 *
 * \param pool     The Voice pool -- must not be \c NULL.
 * \param amount   The number of frames to be mixed.
 * \param offset   The buffer offset.
 * \param freq     The mixing frequency -- must be > \c 0.
 *
 * \return   The number of active Voices.
 */
uint16_t Voice_pool_mix(Voice_pool* pool,
        uint32_t amount,
        uint32_t offset,
        uint32_t freq);


/**
 * Resets all Voices in the Voice pool.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 */
void Voice_pool_reset(Voice_pool* pool);


/**
 * Destroys an existing Voice pool.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 */
void del_Voice_pool(Voice_pool* pool);


#endif // K_VOICE_POOL_H


