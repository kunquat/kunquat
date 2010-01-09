

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#ifndef K_CHANNEL_STATE_H
#define K_CHANNEL_STATE_H


#include <stdbool.h>
#include <stdint.h>

#include <Reltime.h>
#include <kunquat/limits.h>


/**
 * This structure is used for transferring channel-specific settings to
 * Voices.
 */
typedef struct Channel_state
{
    int num;                       ///< Channel number.
    bool* mute;                    ///< Channel mute.

    double volume;                 ///< Channel volume (linear factor).

    Reltime force_slide_length;
    double tremolo_length;         ///< Tremolo length.
    double tremolo_update;         ///< Tremolo update.
    double tremolo_depth;          ///< Tremolo depth.
    double tremolo_delay_update;   ///< The update amount of the tremolo delay.

    Reltime pitch_slide_length;
    double vibrato_length;         ///< Vibrato length.
    double vibrato_update;         ///< Vibrato update.
    double vibrato_depth;          ///< Vibrato depth.
    double vibrato_delay_update;   ///< The update amount of the vibrato delay.

    Reltime filter_slide_length;
    double autowah_length;         ///< Auto-wah length.
    double autowah_update;         ///< Auto-wah update.
    double autowah_depth;          ///< Auto-wah depth.
    double autowah_delay_update;   ///< The update amount of the auto-wah delay.

    double panning;                ///< The current panning.
    int panning_slide;             ///< Panning slide state (0 = no slide, -1 = left, 1 = right).
    Reltime panning_slide_length;
    double panning_slide_target;   ///< Target panning position of the slide.
    double panning_slide_frames;   ///< Number of frames left to complete the slide.
    double panning_slide_update;   ///< The update amount of the slide.
    uint32_t panning_slide_prog;   ///< The amount of frames slided in the Voice processing.
} Channel_state;


/**
 * Initialises the Channel state with default values.
 *
 * \param state   The Channel state -- must not be \c NULL.
 * \param num     The Channel number -- must be >= \c 0 and
 *                < \c KQT_COLUMNS_MAX.
 * \param mute    A reference to the channel mute state -- must not be
 *                \c NULL.
 *
 * \return   The parameter \a state.
 */
Channel_state* Channel_state_init(Channel_state* state, int num, bool* mute);


/**
 * Copies the Channel state.
 *
 * \param dest   The destination Channel state -- must not be \c NULL.
 * \param src    The source Channel state -- must not be \c NULL.
 *
 * \return   The parameter \a dest.
 */
Channel_state* Channel_state_copy(Channel_state* dest, const Channel_state* src);


#endif // K_CHANNEL_STATE_H


