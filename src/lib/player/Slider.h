

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_SLIDER_H
#define KQT_SLIDER_H


#include <mathnum/Tstamp.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * Slider performs a smooth transition of a value.
 */
typedef struct Slider
{
    int32_t audio_rate;
    double tempo;

    Tstamp length;
    double from;
    double to;

    // TODO: these values are part of a temporary solution;
    //       internal slide progress should be a timestamp
    double progress;
    double progress_update;
} Slider;


/**
 * Initialise a Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 *
 * \return   The parameter \a slider.
 */
Slider* Slider_init(Slider* slider);


/**
 * Copy a Slider.
 *
 * \param dest   The destination Slider -- must not be \c NULL.
 * \param src    The source Slider -- must not be \c NULL or \a dest.
 *
 * \return   The parameter \a dest.
 */
Slider* Slider_copy(Slider* restrict dest, const Slider* restrict src);


/**
 * Start a slide.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param target   The target value -- must be finite.
 * \param start    The starting value -- must be finite.
 */
void Slider_start(Slider* slider, double target, double start);


/**
 * Get the current value in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 *
 * \return   The current value.
 */
double Slider_get_value(const Slider* slider);


/**
 * Perform a step in the Slider.
 *
 * It is OK to call this function repeatedly after a slide has finished; the
 * target value will be returned in that case.
 *
 * \param slider   The Slider -- must not be \c NULL.
 *
 * \return   The new intermediate (or target) value in \a slider.
 */
double Slider_step(Slider* slider);


/**
 * Skip a portion of the slide in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param steps    The number of steps to be skipped -- must be >= \c 0.
 *
 * \return   The new intermediate (or target) value in \a slider.
 */
double Slider_skip(Slider* slider, int64_t steps);


/**
 * Estimate the number of active steps left in the Slider.
 *
 * This may be different from the actual number of steps with very slow slides.
 *
 * \param slider   The Slider -- must not be \c NULL.
 *
 * \return   The estimated number of steps left (always positive), or \c 0 if
 *           \a slider is inactive. Note that this value becomes obsolete if
 *           audio rate or tempo changes.
 */
int32_t Slider_estimate_active_steps_left(const Slider* slider);


/**
 * Explicitly break a slide in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 */
void Slider_break(Slider* slider);


/**
 * Set the length of the slide in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param length   The new length -- must not be \c NULL or negative.
 */
void Slider_set_length(Slider* slider, const Tstamp* length);


/**
 * Set the audio rate assumed by the Slider.
 *
 * \param slider       The Slider -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be > \c 0.
 */
void Slider_set_audio_rate(Slider* slider, int32_t audio_rate);


/**
 * Set the tempo in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param tempo    The tempo -- must be > \c 0 and finite.
 */
void Slider_set_tempo(Slider* slider, double tempo);


/**
 * Change the target value in the Slider.
 *
 * This function resets the internal step counter and therefore is equivalent
 * to calling \a Slider_start with the current value as the starting value
 * and with the internal length.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param target   The target value -- must be finite.
 */
void Slider_change_target(Slider* slider, double target);


/**
 * Find out whether a slide is in progress in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 *
 * \return   \c true if a slide is in progress, otherwise \c false.
 */
bool Slider_in_progress(const Slider* slider);


/**
 * Change Slider range without changing current progress.
 *
 * \param slider        The Slider -- must not be \c NULL.
 * \param from_start    Assumed source range start -- must be finite.
 * \param from_end      Assumed source range end -- must be finite.
 * \param to_start      Assumed target range start -- must be finite.
 * \param to_end        Assumed target range end -- must be finite.
 */
void Slider_change_range(
        Slider* slider,
        double from_start,
        double from_end,
        double to_start,
        double to_end);


#endif // KQT_SLIDER_H


