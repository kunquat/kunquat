

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


#ifndef K_LINEAR_CONTROLS_H
#define K_LINEAR_CONTROLS_H


#include <stdint.h>
#include <stdlib.h>

#include <player/Slider.h>
#include <player/Work_buffer.h>
#include <Tstamp.h>


typedef struct Linear_controls
{
    double value;
    Slider slider;
} Linear_controls;


/**
 * Initialise Linear controls.
 *
 * \param lc           The Linear controls -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 * \param tempo        The tempo -- must be positive.
 */
void Linear_controls_init(Linear_controls* lc, int32_t audio_rate, double tempo);


/**
 * Set audio rate of the Linear controls.
 *
 * \param lc           The Linear controls -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 */
void Linear_controls_set_audio_rate(Linear_controls* lc, int32_t audio_rate);


/**
 * Set tempo of the Linear controls.
 *
 * \param lc      The Linear controls -- must not be \c NULL.
 * \param tempo   the tempo -- must be positive.
 */
void Linear_controls_set_tempo(Linear_controls* lc, double tempo);


/**
 * Reset the Linear controls.
 *
 * \param lc   The Linear controls -- must not be \c NULL.
 */
void Linear_controls_reset(Linear_controls* lc);


/**
 * Set the value of the Linear controls.
 *
 * \param lc      The Linear controls -- must not be \c NULL.
 * \param value   The new value -- must be finite.
 */
void Linear_controls_set_value(Linear_controls* lc, double value);


/**
 * Set the target value of the slide in the Linear controls.
 *
 * \param lc      The Linear controls -- must not be \c NULL.
 * \param value   The new target value -- must be finite.
 */
void Linear_controls_slide_value_target(Linear_controls* lc, double value);


/**
 * Set the length of the slide in the Linear controls.
 *
 * \param lc       The Linear controls -- must not be \c NULL.
 * \param length   The length of the slide -- must not be \c NULL.
 */
void Linear_controls_slide_value_length(Linear_controls* lc, const Tstamp* length);


/**
 * Fill Work buffer with updates of the Linear controls.
 *
 * \param lc          The Linear controls -- must not be \c NULL.
 * \param wb          The Work buffer -- must not be \c NULL.
 * \param buf_start   The buffer start index -- must be >= \c 0.
 * \param buf_stop    The buffer stop index -- must be >= \a buf_start.
 */
void Linear_controls_fill_work_buffer(
        Linear_controls* lc,
        const Work_buffer* wb,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Copy the current state of Linear controls.
 *
 * \param dest   The destination Linear controls -- must not be \c NULL.
 * \param src    The source Linear controls -- must not be \c NULL or \a dest.
 */
void Linear_controls_copy(
        Linear_controls* restrict dest, const Linear_controls* restrict src);


#endif // K_LINEAR_CONTROLS_H


