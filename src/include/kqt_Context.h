

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


#ifndef K_CONTEXT_H
#define K_CONTEXT_H


#include <stdint.h>

#include <Playdata.h>
#include <Song.h>
#include <Voice_pool.h>


#define MAX_VOICES (1024)


typedef struct kqt_Context
{
    Song* song;
    Playdata* play;
    int32_t id;
    Voice_pool* voices;
} kqt_Context;


/**
 * Creates a new kqt_Context.
 *
 * \param freq     The mixing frequency -- must be > \c 0.
 * \param voices   The number of Voices -- must be > \c 0 and < \c MAX_VOICES.
 * \param song     The Song -- must not be \c NULL.
 *
 * \return   The new kqt_Context if successful, or \c NULL if memory allocation
 *           failed.
 */
kqt_Context* new_kqt_Context(uint32_t freq, uint16_t voices, Song* song);


/**
 * Gets the ID of the kqt_Context.
 *
 * \param context   The kqt_Context -- must not be \c NULL.
 *
 * \return   The ID.
 */
int32_t kqt_Context_get_id(kqt_Context* context);


/**
 * Gets the Song of the kqt_Context.
 *
 * \param context   The kqt_Context -- must not be \c NULL.
 *
 * \return   The Song.
 */
Song* kqt_Context_get_song(kqt_Context* context);


/**
 * Gets the Playdata of the kqt_Context.
 *
 * \param context   The kqt_Context -- must not be \c NULL.
 *
 * \return   The Playdata.
 */
Playdata* kqt_Context_get_playdata(kqt_Context* context);


/**
 * Does mixing according to the state of the kqt_Context.
 *
 * \param context   The kqt_Context -- must not be \c NULL.
 * \param nframes   The number of frames to be mixed.
 *
 * \return   The number of frames actually mixed. This is always
 *           <= \a nframes.
 */
uint32_t kqt_Context_mix(kqt_Context* context, uint32_t nframes);


/**
 * Plays one Event. The caller should have set the Event in the desired
 * Channel beforehand.
 *
 * \param context   The kqt_Context -- must not be \c NULL.
 */
void kqt_Context_play_event(kqt_Context* context);


/**
 * Plays one Pattern.
 *
 * \param context   The kqt_Context -- must not be \c NULL.
 * \param num       The number of the Pattern -- must be >= \c 0 and
 *                  < \c PATTERNS_MAX.
 * \param tempo     The tempo -- must be > \c 0.
 */
void kqt_Context_play_pattern(kqt_Context* context, int16_t num, double tempo);


/**
 * Plays a subsong.
 *
 * \param context   The kqt_Context -- must not be \c NULL.
 * \param num       The number of the subsong -- must be < \c SUBSONGS_MAX.
 */
void kqt_Context_play_subsong(kqt_Context* context, uint16_t subsong);


/**
 * Plays the default subsong of the Song.
 *
 * \param context   The kqt_Context -- must not be \c NULL.
 */
void kqt_Context_play_song(kqt_Context* context);


/**
 * Stops playback.
 *
 * \param context   The kqt_Context -- must not be \c NULL.
 */
void kqt_Context_stop(kqt_Context* context);


/**
 * Sets a new mixing frequency.
 * 
 * \param context   The kqt_Context -- must not be \c NULL.
 * \param freq      The mixing frequency -- must be > \c 0.
 */
void kqt_Context_set_mix_freq(kqt_Context* context, uint32_t freq);


/**
 * Destroys an existing kqt_Context.
 *
 * The Song inside the context is also destroyed.
 *
 * \param context   The kqt_Context -- must not be \c NULL.
 */
void del_kqt_Context(kqt_Context* context);


#endif // K_CONTEXT_H


