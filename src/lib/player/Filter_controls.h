

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_FILTER_CONTROLS_H
#define K_FILTER_CONTROLS_H


#include <player/LFO.h>
#include <player/Slider.h>

#include <stdint.h>
#include <stdlib.h>


/**
 * Filter parameters that can be carried from one note to the next in a channel.
 */
typedef struct Filter_controls
{
    double lowpass;
    Slider lowpass_slider;
    LFO    autowah;
    double resonance;
    Slider resonance_slider;
} Filter_controls;


/**
 * Initialise Filter controls.
 *
 * \param fc           The Filter controls -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 * \param tempo        the tempo -- must be positive.
 */
void Filter_controls_init(Filter_controls* fc, int32_t audio_rate, double tempo);


/**
 * Set audio rate of the Filter controls.
 *
 * \param fc           The Filter controls -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 */
void Filter_controls_set_audio_rate(Filter_controls* fc, int32_t audio_rate);


/**
 * Set tempo of the Filter controls.
 *
 * \param fc      The Filter controls -- must not be \c NULL.
 * \param tempo   the tempo -- must be positive.
 */
void Filter_controls_set_tempo(Filter_controls* fc, double tempo);


/**
 * Reset Filter controls.
 *
 * \param fc   The Filter controls -- must not be \c NULL.
 */
void Filter_controls_reset(Filter_controls* fc);


/**
 * Copy the current state of Filter controls.
 *
 * \param dest   The destination Filter controls -- must not be \c NULL.
 * \param src    The source Filter controls -- must not be \c NULL or \a dest.
 */
void Filter_controls_copy(
        Filter_controls* restrict dest, const Filter_controls* restrict src);


#endif // K_FILTER_CONTROLS_H


