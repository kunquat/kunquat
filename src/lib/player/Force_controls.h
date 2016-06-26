

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


#ifndef KQT_FORCE_CONTROLS_H
#define KQT_FORCE_CONTROLS_H


#include <player/LFO.h>
#include <player/Slider.h>

#include <stdint.h>
#include <stdlib.h>


/**
 * Force parameters that can be carried from one note to the next in a channel.
 */
typedef struct Force_controls
{
    double force;
    Slider slider;
    LFO    tremolo;
} Force_controls;


/**
 * Initialise Force controls.
 *
 * \param fc           The Force controls -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 * \param tempo        the tempo -- must be positive.
 */
void Force_controls_init(Force_controls* fc, int32_t audio_rate, double tempo);


/**
 * Set audio rate of the Force controls.
 *
 * \param fc           The Force controls -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 */
void Force_controls_set_audio_rate(Force_controls* fc, int32_t audio_rate);


/**
 * Set tempo of the Force controls.
 *
 * \param fc      The Force controls -- must not be \c NULL.
 * \param tempo   the tempo -- must be positive.
 */
void Force_controls_set_tempo(Force_controls* fc, double tempo);


/**
 * Reset Force controls.
 *
 * \param fc   The Force controls -- must not be \c NULL.
 */
void Force_controls_reset(Force_controls* fc);


/**
 * Copy the current state of Force controls.
 *
 * \param dest   The destination Force controls -- must not be \c NULL.
 * \param src    The source Force controls -- must not be \c NULL or \a dest.
 */
void Force_controls_copy(
        Force_controls* restrict dest, const Force_controls* restrict src);


#endif // KQT_FORCE_CONTROLS_H


