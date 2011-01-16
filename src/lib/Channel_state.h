

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
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
#include <General_state.h>
#include <LFO.h>
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
    General_state parent;
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
    int effect;                    ///< Currently active Effect.
    bool inst_effects;             ///< Instrument effects enabled.
    int dsp;                       ///< Currently active DSP.
    int dsp_context;               ///< Currently active DSP context. TODO: remove
    Ins_table* insts;
    uint32_t* freq;
    double* tempo;

    double volume;                 ///< Channel volume (linear factor).

    Reltime force_slide_length;
    LFO tremolo;
    double tremolo_speed;
    Reltime tremolo_speed_delay;
    double tremolo_depth;
    Reltime tremolo_depth_delay;

    Reltime pitch_slide_length;
    LFO vibrato;
    double vibrato_speed;
    Reltime vibrato_speed_delay;
    double vibrato_depth;
    Reltime vibrato_depth_delay;

    Reltime filter_slide_length;
    LFO autowah;
    double autowah_speed;
    Reltime autowah_speed_delay;
    double autowah_depth;
    Reltime autowah_depth_delay;

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


