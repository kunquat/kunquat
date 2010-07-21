

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
 * \param slider     The Slider -- must not be \c NULL.
 * \param mode       The Slide mode -- must be valid.
 * \param mix_rate   The mix rate -- must be > \c 0.
 * \param tempo      The tempo -- must be > \c 0 and finite.
 *
 * \param   The parameter \a slider.
 */
Slider* Slider_init(Slider* slider,
                    Slide_mode mode,
                    uint32_t mix_rate,
                    double tempo);


/**
 * Starts a slide.
 *
 * \param slider    The Slider -- must not be \c NULL.
 * \param current   The starting value -- must be finite.
 * \param target    The target value -- must be finite.
 * \param length    The length of the slide -- must not be \c NULL
 *                  or negative.
 */
void Slider_start(Slider* slider,
                  double start,
                  double target,
                  Reltime* length);


/**
 * Performs a step in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 *
 * \return   The new intermediate (or target) value in \a slider.
 */
double Slider_step(Slider* slider);


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
 * Changes the length of the slide in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param length   The new length -- must not be \c NULL or negative.
 */
void Slider_change_length(Slider* slider, Reltime* length);


/**
 * Changes the mixing rate assumed by the Slider.
 *
 * \param slider     The Slider -- must not be \c NULL.
 * \param mix_rate   The mix rate -- must be > \c 0.
 */
void Slider_change_mix_rate(Slider* slider, uint32_t mix_rate);


/**
 * Changes the tempo in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 * \param tempo    The tempo -- must be > \c 0 and finite.
 */
void Slider_change_tempo(Slider* slider, double tempo);


/**
 * Tells whether the slide target has been reached in the Slider.
 *
 * \param slider   The Slider -- must not be \c NULL.
 *
 * \return   \c true if the target value has been reached, otherwise \c false.
 */
bool Slider_target_reached(Slider* slider);


#endif // K_SLIDER_H


