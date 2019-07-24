

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PLAYER_SEQ_H
#define KQT_PLAYER_SEQ_H


#include <player/Player_private.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


bool get_event_type_info(
        Streader* desc_reader,
        const Event_names* names,
        char* ret_name,
        Event_type* ret_type);


void Player_reset_channels(Player* player);


void Player_process_event(
        Player* player,
        int ch_num,
        const char* event_name,
        const Value* arg,
        bool is_at_global_breakpoint,
        int32_t frame_offset,
        bool skip,
        bool external);


bool Player_check_perform_goto(Player* player);


// TODO: Implement Player_check_perform_jump


void Player_update_sliders_and_lfos_tempo(Player* player);


/**
 * Move the sequencer state forwards.
 *
 * \param player    The Player -- must not be \c NULL and must be in playing state.
 * \param nframes   The number of frames to move forwards -- must be >= \c 0.
 * \param skip      Whether or not event processing should be skipped.
 *
 * \return   The number of frames to be rendered.
 */
int32_t Player_move_forwards(Player* player, int32_t nframes, bool skip);


#endif // KQT_PLAYER_SEQ_H


