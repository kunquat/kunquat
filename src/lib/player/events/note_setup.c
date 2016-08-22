

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/note_setup.h>

#include <debug/assert.h>
#include <init/devices/Audio_unit.h>
#include <kunquat/limits.h>
#include <player/Channel.h>
#include <player/devices/Voice_state.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


void reserve_voice(
        Channel* ch,
        Audio_unit* au,
        uint64_t group_id,
        const Proc_state* proc_state,
        int proc_num,
        uint64_t rand_seed)
{
    rassert(ch != NULL);
    rassert(ch->freq != NULL);
    rassert(*ch->freq > 0);
    rassert(ch->tempo != NULL);
    rassert(*ch->tempo > 0);
    rassert(au != NULL);
    rassert(proc_state != NULL);
    rassert(proc_num >= 0);
    rassert(proc_num < KQT_PROCESSORS_MAX);

    ++ch->fg_count;
    ch->fg[proc_num] = Voice_pool_get_voice(ch->pool, NULL, 0);
    rassert(ch->fg[proc_num] != NULL);
//    fprintf(stderr, "allocated Voice %p\n", (void*)ch->fg[proc_num]);
    ch->fg_id[proc_num] = Voice_id(ch->fg[proc_num]);

    // Get expression settings
    const char* init_expr =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_INIT_EXPRESSION);
    const char* expr = Active_names_get(ch->parent.active_names, ACTIVE_CAT_EXPRESSION);
    rassert(strlen(init_expr) < KQT_VAR_NAME_MAX);
    rassert(strlen(expr) < KQT_VAR_NAME_MAX);

    Voice_init(
            ch->fg[proc_num],
            Audio_unit_get_proc(au, proc_num),
            group_id,
            proc_state,
            rand_seed);

    Voice_state* vstate = ch->fg[proc_num]->state;
    strcpy(vstate->init_expr_name, init_expr);
    if (ch->carry_expression)
        strcpy(vstate->expr_name, expr);

    return;
}


