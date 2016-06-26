

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PITCH_CONTROLS_H
#define KQT_PITCH_CONTROLS_H


#include <player/LFO.h>
#include <player/Slider.h>

#include <stdint.h>
#include <stdlib.h>


/**
 * Pitch parameters that can be carried from one note to the next in a channel.
 */
typedef struct Pitch_controls
{
    double pitch;
    double orig_carried_pitch;
    double pitch_add;
    Slider slider;
    LFO    vibrato;
} Pitch_controls;


/**
 * Initialise Pitch controls.
 *
 * \param pc           The Pitch controls -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 * \param tempo        the tempo -- must be positive.
 */
void Pitch_controls_init(Pitch_controls* pc, int32_t audio_rate, double tempo);


/**
 * Set audio rate of the Pitch controls.
 *
 * \param pc           The Pitch controls -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 */
void Pitch_controls_set_audio_rate(Pitch_controls* pc, int32_t audio_rate);


/**
 * Set tempo of the Pitch controls.
 *
 * \param pc      The Pitch controls -- must not be \c NULL.
 * \param tempo   the tempo -- must be positive.
 */
void Pitch_controls_set_tempo(Pitch_controls* pc, double tempo);


/**
 * Reset Pitch controls.
 *
 * \param pc   The Pitch controls -- must not be \c NULL.
 */
void Pitch_controls_reset(Pitch_controls* pc);


/**
 * Copy the current state of Pitch controls.
 *
 * \param dest   The destination Pitch controls -- must not be \c NULL.
 * \param src    The source Pitch controls -- must not be \c NULL or \a dest.
 */
void Pitch_controls_copy(
        Pitch_controls* restrict dest, const Pitch_controls* restrict src);


#endif // KQT_PITCH_CONTROLS_H


