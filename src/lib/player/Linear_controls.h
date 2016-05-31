

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


#include <decl.h>
#include <mathnum/Tstamp.h>
#include <player/LFO.h>
#include <player/Slider.h>
#include <player/Work_buffer.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


struct Linear_controls
{
    double value;
    double min_value;
    double max_value;
    Slider slider;
    LFO lfo;
};


/**
 * Initialise Linear controls.
 *
 * \param lc   The Linear controls -- must not be \c NULL.
 */
void Linear_controls_init(Linear_controls* lc);


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
 * Set value range in the Linear controls.
 *
 * \param lc          The Linear controls -- must not be \c NULL.
 * \param min_value   The minimum value -- must not be \c NAN.
 * \param max_value   The maximum value -- must be >= \a min_value.
 */
void Linear_controls_set_range(Linear_controls* lc, double min_value, double max_value);


/**
 * Set the value of the Linear controls.
 *
 * \param lc      The Linear controls -- must not be \c NULL.
 * \param value   The new value -- must be finite.
 */
void Linear_controls_set_value(Linear_controls* lc, double value);


/**
 * Get the current value in the Linear controls.
 *
 * NOTE: The returned value does not have oscillation applied.
 *
 * \param lc   The Linear controls -- must not be \c NULL.
 *
 * \return   The current value, or \c NAN if unspecified.
 */
double Linear_controls_get_value(const Linear_controls* lc);


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
 * Set the oscillation speed in the Linear controls.
 *
 * \param lc      The Linear controls -- must not be \c NULL.
 * \param speed   The oscillation speed -- must be >= \c 0.
 */
void Linear_controls_osc_speed_value(Linear_controls* lc, double speed);


/**
 * Set the oscillation depth in the Linear controls.
 *
 * \param lc      The Linear controls -- must not be \c NULL.
 * \param depth   The oscillation depth -- must be finite.
 */
void Linear_controls_osc_depth_value(Linear_controls* lc, double depth);


/**
 * Set the oscillation speed slide in the Linear controls.
 *
 * \param lc       The Linear controls -- must not be \c NULL.
 * \param length   The slide length -- must not be \c NULL.
 */
void Linear_controls_osc_speed_slide_value(Linear_controls* lc, const Tstamp* length);


/**
 * Set the oscillation depth slide in the Linear controls.
 *
 * \param lc       The Linear controls -- must not be \c NULL.
 * \param length   The slide length -- must not be \c NULL.
 */
void Linear_controls_osc_depth_slide_value(Linear_controls* lc, const Tstamp* length);


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
        Work_buffer* wb,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Update internal state of the Linear controls without storing results.
 *
 * \param lc           The Linear controls -- must not be \c NULL.
 * \param step_count   The number of steps to skip.
 */
void Linear_controls_skip(Linear_controls* lc, uint64_t step_count);


/**
 * Copy the current state of Linear controls.
 *
 * \param dest   The destination Linear controls -- must not be \c NULL.
 * \param src    The source Linear controls -- must not be \c NULL or \a dest.
 */
void Linear_controls_copy(
        Linear_controls* restrict dest, const Linear_controls* restrict src);


/**
 * Get Linear controls converted to another range.
 *
 * \param dest         The destination Linear controls -- must not be \c NULL.
 * \param map_min_to   The target value of source range lower bound -- must be finite.
 * \param map_max_to   The target value of source range upper bound -- must be finite.
 * \param src          The source Linear controls -- must not be \c NULL or \a dest.
 * \param range_min    The source range lower bound -- must be finite.
 * \param range_max    The source range upper bound -- must be finite.
 */
void Linear_controls_convert(
        Linear_controls* restrict dest,
        double map_min_to,
        double map_max_to,
        const Linear_controls* restrict src,
        double range_min,
        double range_max);


#endif // K_LINEAR_CONTROLS_H


