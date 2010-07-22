

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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

#include <Reltime.h>


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
    uint32_t mix_rate;
    double tempo;

    int dir;
    Reltime length;
    double current_value;
    double target_value;
    double steps_left;
    double update;
} Slider;


/**
 * Initialises a Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param mode     The Slide mode -- must be valid.
 *
 * \return   The parameter \a slider.
 */
Slider* Slider_init(Slider* slider, Slide_mode mode);


/**
 * Copies a Slider.
 *
 * \param dest   The destination Slider -- must not be \c NULL.
 * \param src    The source Slider -- must not be \c NULL or \a dest.
 *
 * \return   The parameter \a dest.
 */
Slider* Slider_copy(Slider* restrict dest, const Slider* restrict src);


/**
 * Starts a slide.
 *
 * \param slider     The Slider -- must not be \c NULL.
 * \param target     The target value -- must be finite.
 * \param start      The starting value -- must be finite.
 */
void Slider_start(Slider* slider,
                  double target,
                  double start);


/**
 * Performs a step in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 *
 * \return   The new intermediate (or target) value in \a slider.
 */
double Slider_step(Slider* slider);


/**
 * Skips a portion of the slide in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param steps    The number of steps to be skipped.
 *
 * \return   The new intermediate (or target) value in \a slider.
 */
double Slider_skip(Slider* slider, uint64_t steps);


/**
 * Explicitly breaks a slide in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 */
void Slider_break(Slider* slider);


/**
 * Sets the length of the slide in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param length   The new length -- must not be \c NULL or negative.
 */
void Slider_set_length(Slider* slider, Reltime* length);


/**
 * Sets the mixing rate assumed by the Slider.
 *
 * \param slider     The Slider -- must not be \c NULL.
 * \param mix_rate   The mix rate -- must be > \c 0.
 */
void Slider_set_mix_rate(Slider* slider, uint32_t mix_rate);


/**
 * Sets the tempo in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param tempo    The tempo -- must be > \c 0 and finite.
 */
void Slider_set_tempo(Slider* slider, double tempo);


/**
 * Changes the target value in the Slider.
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
 * Finds out whether a slide is in progress in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 *
 * \return   \c true if a slide is in progress, otherwise \c false.
 */
bool Slider_in_progress(Slider* slider);


#endif // K_SLIDER_H


