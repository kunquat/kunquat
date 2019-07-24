

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2019
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
#include <init/devices/Device.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Param_proc_filter.h>
#include <init/Module.h>
#include <kunquat/limits.h>
#include <player/Channel.h>
#include <player/devices/Voice_state.h>
#include <player/Voice.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static bool reserve_voice(
        Channel* ch, uint64_t group_id, const Proc_state* proc_state, bool is_external)
{
    rassert(ch != NULL);
    rassert(proc_state != NULL);

    if (!Proc_state_needs_vstate(proc_state))
        return false;

    Voice* voice = Voice_pool_allocate_voice(ch->pool, ch->num, group_id, is_external);
    ignore(voice);
    //fprintf(stderr, "reserved Voice %p\n", (void*)voice);

    return true;
}


bool reserve_voices(
        Channel* ch,
        const Module* module,
        const Device_states* dstates,
        Event_type event_type,
        const Value* arg,
        bool is_external)
{
    rassert(ch != NULL);
    rassert(module != NULL);
    rassert(dstates != NULL);
    rassert(arg != NULL);

    int reserve_count = 0;

    if (event_type == Event_channel_note_on)
    {
        rassert(arg->type == VALUE_TYPE_FLOAT);

        Audio_unit* au = Module_get_au_from_input(module, ch->au_input);
        if ((au != NULL) && (Audio_unit_get_type(au) == AU_TYPE_INSTRUMENT))
        {
            const uint64_t new_group_id = Voice_pool_new_group_id(ch->pool);

            for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
            {
                const Processor* proc = Audio_unit_get_proc(au, i);
                if (proc == NULL ||
                        !Device_is_existent((const Device*)proc) ||
                        !Processor_get_voice_signals(proc))
                    continue;

                const Proc_state* proc_state = (Proc_state*)Device_states_get_state(
                        dstates, Device_get_id((const Device*)proc));

                if (reserve_voice(ch, new_group_id, proc_state, is_external))
                    ++reserve_count;
            }

            Voice_group_reservations_add_entry(
                    ch->voice_group_res, ch->num, new_group_id);
        }
    }
    else if (event_type == Event_channel_hit)
    {
        rassert(arg->type == VALUE_TYPE_INT);

        Audio_unit* au = Module_get_au_from_input(module, ch->au_input);
        if ((au != NULL) && (Audio_unit_get_type(au) == AU_TYPE_INSTRUMENT))
        {
            const int hit_index = (int)arg->value.int_type;
            if (Audio_unit_get_hit_existence(au, hit_index))
            {
                const Param_proc_filter* hpf =
                    Audio_unit_get_hit_proc_filter(au, hit_index);

                const uint64_t new_group_id = Voice_pool_new_group_id(ch->pool);

                for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
                {
                    const Processor* proc = Audio_unit_get_proc(au, i);
                    if (proc == NULL ||
                            !Device_is_existent((const Device*)proc) ||
                            !Processor_get_voice_signals(proc))
                        continue;

                    // Skip processors that are filtered out for this hit index
                    if ((hpf != NULL) && !Param_proc_filter_is_proc_allowed(hpf, i))
                        continue;

                    const Proc_state* proc_state = (Proc_state*)Device_states_get_state(
                            dstates, Device_get_id((const Device*)proc));

                    if (reserve_voice(ch, new_group_id, proc_state, is_external))
                        ++reserve_count;
                }

                Voice_group_reservations_add_entry(
                        ch->voice_group_res, ch->num, new_group_id);
            }
        }
    }
    else
    {
        rassert(false);
    }

    return (reserve_count > 0);
}


bool init_voice(
        Channel* ch,
        Voice* voice,
        const Audio_unit* au,
        uint64_t group_id,
        const Proc_state* proc_state,
        int proc_num,
        uint64_t rand_seed)
{
    rassert(ch != NULL);
    rassert(ch->audio_rate > 0);
    rassert(ch->tempo > 0);
    rassert(voice != NULL);
    rassert(au != NULL);
    rassert(proc_state != NULL);
    rassert(proc_num >= 0);
    rassert(proc_num < KQT_PROCESSORS_MAX);

    if (Voice_get_group_id(voice) != group_id)
        return false;

    //fprintf(stderr, "initialised Voice %p\n", (void*)voice);

    // Get expression settings
    const char* ch_expr =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CH_EXPRESSION);
    const char* note_expr =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_NOTE_EXPRESSION);
    rassert(strlen(ch_expr) <= KQT_VAR_NAME_MAX);
    rassert(strlen(note_expr) <= KQT_VAR_NAME_MAX);

    Voice_start(
            voice,
            Audio_unit_get_proc(au, proc_num),
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

    return true;
}


