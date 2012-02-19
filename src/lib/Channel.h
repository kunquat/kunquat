

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_CHANNEL_H
#define K_CHANNEL_H


#include <stdint.h>

#include <Channel_state.h>
#include <Voice.h>
#include <Event.h>
#include <Event_cache.h>
#include <Event_handler.h>
#include <Voice_pool.h>
#include <Ins_table.h>
#include <Column.h>
#include <kunquat/limits.h>


/**
 * This object contains playback information of a channel. A single channel
 * may contain 0 or 1 foreground (fully controllable) Voices -- background
 * Voices are only present in the Voice pool.
 */
typedef struct Channel
{
    Channel_state init_state; ///< Channel state at the start of the playback.
    Channel_state cur_state;  ///< Channel state as passed to Voices.
//    Channel_state new_state;  ///< Channel state as received from Voices.
    bool mute;                ///< If \c true, output of the Voices will be ignored.
//    int cur_inst;             ///< Current instrument number.
//    Ins_table* insts; ///< The Instrument table.
//    int fg_count; ///< Number of Voices in the foreground.
//    Voice* fg[KQT_GENERATORS_MAX]; ///< The Voices in the foreground.
//    uint64_t fg_id[KQT_GENERATORS_MAX]; ///< The reservation identifications.
    Event* note_off; ///< A Note Off event for triggering implicit Note Offs.
//    Event* single; ///< An Event used for single note playback control.
} Channel;


/**
 * Creates a new Channel.
 *
 * \param insts        The Instrument table of the Song -- must not be \c NULL.
 * \param num          The Channel number -- must be >= \c 0 and
 *                     < \c KQT_COLUMNS_MAX.
 * \param pool         The Voice pool -- must not be \c NULL.
 * \param env          The Environment -- must not be \c NULL.
 * \param tempo        A reference to the tempo -- must not be \c NULL.
 * \param freq         A reference to the mix rate -- must not be \c NULL.
 *
 * \return   The new Channel if successful, or \c NULL if memory allocation
 *           failed.
 */
Channel* new_Channel(Ins_table* insts,
                     int num,
                     Voice_pool* pool,
                     Environment* env,
                     double* tempo,
                     uint32_t* freq);


/**
 * Mixes the foreground of the Channel.
 *
 * \param ch        The Channel -- must not be \c NULL.
 * \param pool      The Voice pool -- must not be \c NULL.
 * \param nframes   Upper limit in the buffer to be mixed.
 * \param offset    Buffer offset -- must be <= \a nframes.
 * \param tempo     The tempo -- must be > \c 0.
 * \param freq      The mixing rate -- must be > \c 0.
 */
void Channel_mix(Channel* ch,
                 Voice_pool* pool,
                 uint32_t nframes,
                 uint32_t offset,
                 double tempo,
                 uint32_t freq);


/**
 * Updates playback state of the Channel.
 *
 * \param ch      The Channel -- must not be \c NULL.
 * \param mixed   Number of frames mixed.
 */
void Channel_update_state(Channel* ch, uint32_t mixed);


/**
 * Sets the random seed of the Channel.
 *
 * \param ch     The Channel -- must not be \c NULL.
 * \param seed   The seed.
 */
void Channel_set_random_seed(Channel* ch, uint64_t seed);


/**
 * Sets the Event cache of the Channel.
 *
 * \param ch      The Channel -- must not be \c NULL.
 * \param cache   The Event cache -- must not be \c NULL.
 */
void Channel_set_event_cache(Channel* ch, Event_cache* cache);


/**
 * Resets the Channel.
 *
 * \param ch   The Channel -- must not be \c NULL.
 */
void Channel_reset(Channel* ch);


/**
 * Destroys an existing Channel.
 *
 * \param ch   The Channel, or \c NULL.
 */
void del_Channel(Channel* ch);


#endif // K_CHANNEL_H


