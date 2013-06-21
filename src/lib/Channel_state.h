

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <Environment.h>
#include <Event_cache.h>
#include <General_state.h>
#include <LFO.h>
#include <Random.h>
#include <Tstamp.h>
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
    Random* rand;                  ///< Random source for this channel.
    Event_cache* event_cache;

    Voice_pool* pool;              ///< All Voices.
    Voice* fg[KQT_GENERATORS_MAX]; ///< Foreground Voices.
    uint64_t fg_id[KQT_GENERATORS_MAX]; ///< Voice reservation IDs.
    int fg_count;

    int instrument;                ///< Currently active Instrument.
    int generator;                 ///< Currently active Generator.
    int effect;                    ///< Currently active Effect.
    bool inst_effects;             ///< Instrument effect control enabled.
    int dsp;                       ///< Currently active DSP.
    Ins_table* insts;
    uint32_t* freq;
    double* tempo;

    double volume;                 ///< Channel volume (linear factor).

    Tstamp force_slide_length;
    LFO tremolo;
    double tremolo_speed;
    Tstamp tremolo_speed_delay;
    double tremolo_depth;
    Tstamp tremolo_depth_delay;

    Tstamp pitch_slide_length;
    LFO vibrato;
    double vibrato_speed;
    Tstamp vibrato_speed_delay;
    double vibrato_depth;
    Tstamp vibrato_depth_delay;

    Tstamp filter_slide_length;
    LFO autowah;
    double autowah_speed;
    Tstamp autowah_speed_delay;
    double autowah_depth;
    Tstamp autowah_depth_delay;

    double panning;                ///< The current panning.
    Slider panning_slider;

    double arpeggio_ref;
    double arpeggio_speed;
    int arpeggio_edit_pos;
    double arpeggio_tones[KQT_ARPEGGIO_NOTES_MAX];
} Channel_state;


/**
 * Initialises the Channel state with default values.
 *
 * \param state   The Channel state -- must not be \c NULL.
 * \param num     The Channel number -- must be >= \c 0 and
 *                < \c KQT_COLUMNS_MAX.
 * \param mute    A reference to the channel mute state -- must not be
 *                \c NULL.
 * \param env     The Environment -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Channel_state_init(Channel_state* state, int num, bool* mute,
                        Environment* env);


/**
 * Sets the Channel random seed.
 *
 * \param state   The Channel state -- must not be \c NULL.
 * \param seed    The random seed.
 */
void Channel_state_set_random_seed(Channel_state* state, uint64_t seed);


/**
 * Sets the Event cache of the Channel state.
 *
 * \param state   The Channel state -- must not be \c NULL.
 * \param cache   The Event cache -- must not be \c NULL.
 */
void Channel_state_set_event_cache(Channel_state* state, Event_cache* cache);


/**
 * Resets the Channel state.
 *
 * \param state   The Channel state -- must not be \c NULL.
 */
void Channel_state_reset(Channel_state* state);


/**
 * Makes a shallow copy of the Channel state.
 *
 * \param dest   The destination Channel state -- must not be \c NULL.
 * \param src    The source Channel state -- must not be \c NULL.
 *
 * \return   The parameter \a dest.
 */
Channel_state* Channel_state_copy(Channel_state* dest, const Channel_state* src);


/**
 * Returns an actual force of a current foreground Voice.
 *
 * \param state       The Channel state -- must not be \c NULL.
 * \param gen_index   The Generator index -- must be >= \c 0 and
 *                    < \c KQT_GENERATORS_MAX.
 *
 * \return   The actual force if the active foreground Voice at \a gen_index
 *           exists, otherwise NAN.
 */
double Channel_state_get_fg_force(Channel_state* state, int gen_index);


/**
 * Uninitialises the Channel state.
 *
 * \param state   The Channel state, or \c NULL.
 */
void Channel_state_uninit(Channel_state* state);


#endif // K_CHANNEL_STATE_H


