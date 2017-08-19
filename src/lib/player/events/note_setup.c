

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2017
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
#include <init/devices/Au_expressions.h>
#include <init/devices/Audio_unit.h>
#include <kunquat/limits.h>
#include <player/Channel.h>
#include <player/devices/Voice_state.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


void reserve_voice(
        Channel* ch,
        const Audio_unit* au,
        uint64_t group_id,
        const Proc_state* proc_state,
        int proc_num,
        uint64_t rand_seed)
{
    rassert(ch != NULL);
    rassert(ch->audio_rate > 0);
    rassert(ch->tempo > 0);
    rassert(au != NULL);
    rassert(proc_state != NULL);
    rassert(proc_num >= 0);
    rassert(proc_num < KQT_PROCESSORS_MAX);

    ++ch->fg_count;
    ch->fg[proc_num] = Voice_pool_get_voice(ch->pool, NULL, 0);
//    fprintf(stderr, "allocated Voice %p\n", (void*)ch->fg[proc_num]);

    Voice* voice = ch->fg[proc_num];
    rassert(voice != NULL);
    ch->fg_id[proc_num] = Voice_id(voice);

    // Get expression settings
    const char* ch_expr =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CH_EXPRESSION);
    const char* note_expr =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_NOTE_EXPRESSION);
    rassert(strlen(ch_expr) < KQT_VAR_NAME_MAX);
    rassert(strlen(note_expr) < KQT_VAR_NAME_MAX);

    Voice_init(
            voice,
            Audio_unit_get_proc(au, proc_num),
            group_id,
            ch->num,
            proc_state,
            rand_seed);

    // Test voice
    if (ch->use_test_output)
    {
        Voice_set_test_processor(voice, ch->test_proc_index);
        if (proc_num == ch->test_proc_index)
            Voice_set_test_processor_param(voice, ch->test_proc_param);
    }

    // Apply expression settings
    Voice_state* vstate = voice->state;
    strcpy(vstate->ch_expr_name, ch_expr);
    if (ch->carry_note_expression && (note_expr[0] != '\0'))
    {
        strcpy(vstate->note_expr_name, note_expr);
    }
    else
    {
        const Au_expressions* ae = Audio_unit_get_expressions(au);
        if (ae != NULL)
            strcpy(vstate->note_expr_name, Au_expressions_get_default_note_expr(ae));
    }

    return;
}


