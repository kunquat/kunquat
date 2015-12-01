

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SLIDER_H
#define K_SLIDER_H


#include <stdbool.h>
#include <stdint.h>

#include <Tstamp.h>


typedef enum
{
    SLIDE_MODE_LINEAR = 0,
    SLIDE_MODE_EXP
} Slide_mode;


/**
 * Slider performs a smooth transition of a value.
 */
typedef struct Slider
{
    Slide_mode mode;
    int32_t audio_rate;
    double tempo;

    Tstamp length;
    double from;
    double to;

    // TODO: these values are part of a temporary solution;
    //       internal slide progress should be a timestamp
    double progress;
    double progress_update;
    double log2_from;
    double log2_to;
} Slider;


/**
 * Initialise a Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param mode     The Slide mode -- must be valid.
 *
 * \return   The parameter \a slider.
 */
Slider* Slider_init(Slider* slider, Slide_mode mode);


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
 * \param steps    The number of steps to be skipped.
 *
 * \return   The new intermediate (or target) value in \a slider.
 */
double Slider_skip(Slider* slider, uint64_t steps);


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
void Slider_set_mix_rate(Slider* slider, int32_t audio_rate);


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


#endif // K_SLIDER_H


