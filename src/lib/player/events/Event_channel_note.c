

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_channel_decl.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <mathnum/Tstamp.h>
#include <module/Input_map.h>
#include <module/Module.h>
#include <module/Scale.h>
#include <player/events/Event_common.h>
#include <player/events/note_setup.h>
#include <player/Voice.h>
#include <Value.h>

#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


bool Event_channel_note_on_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(ch->freq != NULL);
    assert(*ch->freq > 0);
    assert(ch->tempo != NULL);
    assert(*ch->tempo > 0);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    // Move the old Voices to the background
    Event_channel_note_off_process(ch, dstates, NULL);

    ch->fg_count = 0;

    // Find our audio unit
    Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if (au == NULL)
        return true;

    // Allocate new Voices
//    ch->panning_slide = 0;
    double force_var = NAN;

    bool is_voice_rand_seed_set = false;
    uint64_t voice_rand_seed = 0;

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

        if (!is_voice_rand_seed_set)
        {
            voice_rand_seed = Random_get_uint64(ch->rand);
            is_voice_rand_seed_set = true;
        }

        reserve_voice(ch, au, new_group_id, proc_state, i, voice_rand_seed);

        Voice* voice = ch->fg[i];
        Voice_state* vs = voice->state;

#if 0
        else if (voice->proc->au_params->scale == NULL ||
                 *voice->proc->au_params->scale == NULL ||
                 **voice->proc->au_params->scale == NULL)
        {
            vs->pitch = exp2(value->value.float_type / 1200) * 440;
        }
        else
        {
            pitch_t pitch = Scale_get_pitch_from_cents(
                    **voice->proc->au_params->scale,
                    value->value.float_type);
            if (pitch > 0)
            {
                vs->pitch = pitch;
            }
            else
            {
                vs->pitch = exp2(value->value.float_type / 1200) * 440;
            }
        }
#endif

        vs->orig_pitch_param = value->value.float_type;

        if (ch->carry_pitch)
        {
            if (isnan(ch->pitch_controls.pitch))
                ch->pitch_controls.pitch = exp2(vs->orig_pitch_param / 1200) * 440;
            if (isnan(ch->pitch_controls.orig_carried_pitch))
                ch->pitch_controls.orig_carried_pitch = vs->orig_pitch_param;

            const double pitch_diff =
                vs->orig_pitch_param - ch->pitch_controls.orig_carried_pitch;
            if (pitch_diff != 0)
                ch->pitch_controls.freq_mul = exp2(pitch_diff / 1200);
            else
                ch->pitch_controls.freq_mul = 1;

            Pitch_controls_copy(&vs->pitch_controls, &ch->pitch_controls);
        }
        else
        {
            vs->pitch_controls.pitch = exp2(vs->orig_pitch_param / 1200) * 440;
            vs->pitch_controls.orig_carried_pitch = vs->orig_pitch_param;

            Slider_set_length(&vs->pitch_controls.slider, &ch->pitch_slide_length);

            Pitch_controls_copy(&ch->pitch_controls, &vs->pitch_controls);
        }

        //fprintf(stderr, "Event set pitch @ %p: %f\n", (void*)&vs->pitch, vs->pitch);

        set_au_properties(voice, vs, ch, &force_var);
    }

    Device_init_control_vars(
            (const Device*)au,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            Channel_get_random_source(ch),
            ch);

    return true;
}


bool Event_channel_hit_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(ch->freq != NULL);
    assert(ch->tempo != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_INT);

    // Move the old Voices to the background
    Event_channel_note_off_process(ch, dstates, NULL);

    ch->fg_count = 0;

    // Find our audio unit
    Audio_unit* au = Module_get_au_from_input(
            ch->parent.module,
            ch->au_input);
    if (au == NULL)
        return true;

    double force_var = NAN;

    bool is_voice_rand_seed_set = false;
    uint64_t voice_rand_seed = 0;

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

        if (!is_voice_rand_seed_set)
        {
            voice_rand_seed = Random_get_uint64(ch->rand);
            is_voice_rand_seed_set = true;
        }

        reserve_voice(ch, au, new_group_id, proc_state, i, voice_rand_seed);

        Voice* voice = ch->fg[i];
        Voice_state* vs = voice->state;
        vs->hit_index = value->value.int_type;
        set_au_properties(voice, vs, ch, &force_var);
    }

    Device_init_control_vars(
            (const Device*)au,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            Channel_get_random_source(ch),
            ch);

    return true;
}


bool Event_channel_note_off_process(
        Channel* ch,
        Device_states* dstates,
        const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    ignore(value);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        if (ch->fg[i] != NULL)
        {
            ch->fg[i] = Voice_pool_get_voice(
                    ch->pool,
                    ch->fg[i],
                    ch->fg_id[i]);
            if (ch->fg[i] == NULL)
            {
                // The Voice has been given to another channel
                continue;
            }
            ch->fg[i]->state->note_on = false;
            ch->fg[i]->prio = VOICE_PRIO_BG;
            ch->fg[i] = NULL;
        }
    }
    ch->fg_count = 0;

    return true;
}


