

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PLAYER_SEQ_H
#define K_PLAYER_SEQ_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <player/Player_private.h>


char* get_event_type_info(
        char* desc,
        const Event_names* names,
        Read_state* rs,
        char* ret_name,
        Event_type* ret_type);


void Player_process_trigger(
        Player* player,
        int ch_num,
        char* trigger_desc,
        bool skip);


void Player_process_cgiters(Player* player, Tstamp* limit, bool skip);


void Player_update_sliders_and_lfos_tempo(Player* player);


int32_t Player_move_forwards(Player* player, int32_t nframes, bool skip);


#endif // K_PLAYER_SEQ_H


