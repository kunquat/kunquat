

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


#ifndef K_CHANNEL_STATE_H
#define K_CHANNEL_STATE_H


#include <stdbool.h>
#include <stdint.h>

#include <Channel_gen_state.h>
#include <Reltime.h>
#include <kunquat/limits.h>
#include <Voice_params.h>
#include <Voice_pool.h>
#include <Ins_table.h>


/**
 * This structure is used for transferring channel-specific settings to
 * Voices.
 */
typedef struct Channel_state
{
    int num;                       ///< Channel number.
    Voice_params vp;               ///< Voice parameters.
    bool* mute;                    ///< Channel mute.
    Channel_gen_state* cgstate;    ///< Channel-specific generator state.

    Voice_pool* pool;              ///< All Voices.
    Voice* fg[KQT_GENERATORS_MAX]; ///< Foreground Voices.
    uint64_t fg_id[KQT_GENERATORS_MAX]; ///< Voice reservation IDs.
    int fg_count;

    int instrument;                ///< Currently active Instrument.
    int generator;                 ///< Currently active Generator.
    int dsp;                       ///< Currently active DSP.
    int dsp_context;               ///< Currently active DSP context.
    Ins_table* insts;
    uint32_t* freq;
    double* tempo;

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
    Slider panning_slider;
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
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Channel_state_init(Channel_state* state, int num, bool* mute);


/**
 * Copies the Channel state.
 *
 * \param dest   The destination Channel state -- must not be \c NULL.
 * \param src    The source Channel state -- must not be \c NULL.
 *
 * \return   The parameter \a dest.
 */
Channel_state* Channel_state_copy(Channel_state* dest, const Channel_state* src);


/**
 * Uninitialises the Channel state.
 *
 * \param state   The Channel state, or \c NULL.
 */
void Channel_state_uninit(Channel_state* state);


#endif // K_CHANNEL_STATE_H


