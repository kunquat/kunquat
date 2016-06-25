

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_LFO_H
#define KQT_LFO_H


#include <mathnum/Tstamp.h>
#include <player/Slider.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef enum
{
    LFO_MODE_LINEAR = 0,
    LFO_MODE_EXP
} LFO_mode;


/**
 * LFO (= Low-Frequency Oscillator) is used for performing vibration effects.
 */
typedef struct LFO
{
    LFO_mode mode;
    int32_t audio_rate;
    double tempo;

    bool on;

    double target_speed;
    double prev_speed;
    Slider speed_slider;

    double target_depth;
    double prev_depth;
    Slider depth_slider;

    double offset;
    double phase;
    double update;
} LFO;


/**
 * Initialise a Low-Frequency Oscillator.
 *
 * \param lfo    The LFO -- must not be \c NULL.
 * \param mode   The LFO mode -- must be a valid mode.
 *
 * \return   The parameter \a lfo.
 */
LFO* LFO_init(LFO* lfo, LFO_mode mode);


/**
 * Copy a Low-Frequency Oscillator.
 *
 * \param dest   The destination LFO -- must not be \c NULL.
 * \param src    The source LFO -- must not be \c NULL or \a dest.
 *
 * \return   The parameter \a dest.
 */
LFO* LFO_copy(LFO* restrict dest, const LFO* restrict src);


/**
 * Set the audio rate in the LFO.
 *
 * \param lfo          The LFO -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be > \c 0.
 */
void LFO_set_audio_rate(LFO* lfo, int32_t audio_rate);


/**
 * Set the tempo in the LFO.
 *
 * \param lfo     The LFO -- must not be \c NULL.
 * \param tempo   The tempo -- must be finite and > \c 0.
 */
void LFO_set_tempo(LFO* lfo, double tempo);


/**
 * Set the speed in the LFO.
 *
 * \param lfo     The LFO -- must not be \c NULL.
 * \param speed   The speed -- must be finite and >= \c 0.
 */
void LFO_set_speed(LFO* lfo, double speed);


/**
 * Set the transition slide length for the speed setting of the LFO.
 *
 * \param lfo      The LFO -- must not be \c NULL.
 * \param length   The slide length -- must not be \c NULL or negative.
 */
void LFO_set_speed_slide(LFO* lfo, const Tstamp* length);


/**
 * Set the depth in the LFO.
 *
 * After the depth delay, the LFO will return values in the range
 * [-depth, depth].
 *
 * \param lfo     The LFO -- must not be \c NULL.
 * \param depth   The depth -- must be finite.
 */
void LFO_set_depth(LFO* lfo, double depth);


/**
 * Set the transition slide length for the depth setting of the LFO.
 *
 * \param lfo      The LFO -- must not be \c NULL.
 * \param length   The slide length -- must not be \c NULL or negative.
 */
void LFO_set_depth_slide(LFO* lfo, const Tstamp* length);


/**
 * Set the offset in the LFO.
 *
 * \param lfo      The LFO -- must not be \c NULL.
 * \param offset   The offset -- must be >= \c -1 and <= \c 1.
 */
void LFO_set_offset(LFO* lfo, double offset);


/**
 * Turn on the LFO.
 *
 * \param lfo   The LFO -- must not be \c NULL.
 */
void LFO_turn_on(LFO* lfo);


/**
 * Turn off the LFO.
 *
 * Note that the LFO still provides a smooth ending of the oscillation after
 * turning it off.
 *
 * \param lfo   The LFO -- must not be \c NULL.
 */
void LFO_turn_off(LFO* lfo);


/**
 * Perform a step in the LFO.
 *
 * It is OK to call this function repeatedly after deactivation; the neutral
 * oscillation value will be returned in that case.
 *
 * \param lfo   The LFO -- must not be \c NULL.
 *
 * \return   The new value in the LFO.
 */
double LFO_step(LFO* lfo);


/**
 * Skip a number of steps in the LFO.
 *
 * \param lfo     The LFO -- must not be \c NULL.
 * \param steps   The number of steps.
 *
 * \return   The new value in the LFO.
 */
double LFO_skip(LFO* lfo, uint64_t steps);


/**
 * Find out whether the LFO is still providing non-trivial values.
 *
 * The LFO remains active for some time after turning it off.
 *
 * \param lfo   The LFO -- must not be \c NULL.
 *
 * \return   \c true if \a lfo is active, otherwise \c false.
 */
bool LFO_active(const LFO* lfo);


/**
 * Estimate the number of active steps left in the LFO.
 *
 * This may be different from the actual number of steps with very slow depth
 * slides.
 *
 * \param lfo   The LFO -- must not be \c NULL.
 *
 * \return   The estimated number of steps left (always positive), or \c 0 if
 *           \a lfo is inactive. Note that this value becomes obsolete if
 *           audio rate or tempo changes.
 */
int32_t LFO_estimate_active_steps_left(const LFO* lfo);


/**
 * Return the current target speed of the LFO.
 *
 * \param lfo   The LFO -- must not be \c NULL.
 *
 * \return   The current target speed.
 */
double LFO_get_target_speed(const LFO* lfo);


/**
 * Return the current target depth of the LFO.
 *
 * \param lfo   The LFO -- must not be \c NULL.
 *
 * \return   The current target depth.
 */
double LFO_get_target_depth(const LFO* lfo);


/**
 * Change the depth range of the LFO without resetting internal progress.
 *
 * \param lfo          The LFO -- must not be \c NULL.
 * \param from_depth   The source maximum depth -- must be finite.
 * \param to_depth     The target maximum depth -- must be finite.
 */
void LFO_change_depth_range(LFO* lfo, double from_depth, double to_depth);


#endif // KQT_LFO_H


