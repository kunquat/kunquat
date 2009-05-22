

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


#ifndef K_CHANNEL_H
#define K_CHANNEL_H


#include <stdint.h>

#include <Voice.h>
#include <Event.h>
#include <Voice_pool.h>
#include <Ins_table.h>
#include <Column.h>
#include <Song_limits.h>


/**
 * This object contains playback information of a channel. A single channel
 * may contain 0 or 1 foreground (fully controllable) Voices -- background
 * Voices are only present in the Voice pool.
 */
typedef struct Channel
{
    Ins_table* insts; ///< The Instrument table.
    int fg_count; ///< Number of Voices in the foreground.
    Voice* fg[GENERATORS_MAX]; ///< The Voices in the foreground.
    uint64_t fg_id[GENERATORS_MAX]; ///< The reservation identifications.
    Event* note_off; ///< A Note Off event for triggering implicit Note Offs.
    Event* single; ///< An Event used for single note playback control.
} Channel;


/**
 * Creates a new Channel.
 *
 * \param insts   The Instrument table of the Song -- must not be \c NULL.
 *
 * \return   The new Channel if successful, or \c NULL if memory allocation
 *           failed.
 */
Channel* new_Channel(Ins_table* insts);


/**
 * Set up Voice(s) for events within the interval [start, end).
 *
 * \param ch       The Channel -- must not be \c NULL.
 * \param pool     The Voice pool -- must not be \c NULL.
 * \param citer    The Column iterator -- must not be \c NULL.
 * \param start    The timestamp for first possible Event(s) to be included
 *                 -- must not be \c NULL.
 * \param end      The timestamp at and after which no Events will be
 *                 included, i.e. an Event with precisely or greater than this
 *                 timestamp is left out -- must not be \c NULL.
 * \param offset   The mixing buffer offset.
 * \param tempo    The tempo -- must be > \c 0.
 * \param freq     The mixing frequency -- must be > \c 0.
 */
void Channel_set_voices(Channel* ch,
        Voice_pool* pool,
        Column_iter* citer,
        Reltime* start,
        Reltime* end,
        uint32_t offset,
        double tempo,
        uint32_t freq);


/**
 * Resets the Channel.
 *
 * \param ch   The Channel -- must not be \c NULL.
 */
void Channel_reset(Channel* ch);


/**
 * Destroys an existing Channel
 *
 * \param ch   The Channel -- must not be \c NULL.
 */
void del_Channel(Channel* ch);


#endif // K_CHANNEL_H


