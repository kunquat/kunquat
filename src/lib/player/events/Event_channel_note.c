

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
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
#include <init/devices/Au_streams.h>
#include <init/devices/Param_proc_filter.h>
#include <init/devices/Proc_type.h>
#include <init/Input_map.h>
#include <init/Module.h>
#include <init/Tuning_table.h>
#include <kunquat/limits.h>
#include <mathnum/conversions.h>
#include <mathnum/Tstamp.h>
#include <player/Channel.h>
#include <player/devices/processors/Force_state.h>
#include <player/devices/processors/Pitch_state.h>
#include <player/devices/processors/Stream_state.h>
#include <player/devices/Voice_state.h>
#include <player/events/Event_common.h>
#include <player/events/Event_params.h>
#include <player/events/note_setup.h>
#include <player/events/stream_utils.h>
#include <player/Master_params.h>
#include <player/Tuning_state.h>
#include <player/Voice.h>
#include <Value.h>

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


static void init_force_controls(Channel* ch, const Master_params* master_params)
{
    rassert(ch != NULL);
    rassert(master_params != NULL);

    if (!ch->carry_force || isnan(ch->force_controls.force))
    {
        Force_controls_reset(&ch->force_controls);
        ch->force_controls.force = 0;

        Slider_set_tempo(&ch->force_controls.slider, master_params->tempo);
        Slider_set_length(&ch->force_controls.slider, &ch->force_slide_length);

        LFO_set_tempo(&ch->force_controls.tremolo, master_params->tempo);
        LFO_set_speed_slide(&ch->force_controls.tremolo, &ch->tremolo_speed_slide);
        LFO_set_depth_slide(&ch->force_controls.tremolo, &ch->tremolo_depth_slide);
    }

    return;
}


static void init_streams(Channel* ch, const Audio_unit* au)
{
    const Au_streams* streams = Audio_unit_get_streams(au);
    if (streams != NULL)
    {
        Channel_stream_state* stream_state = Channel_get_stream_state_mut(ch);

        Stream_target_dev_iter* iter =
            Stream_target_dev_iter_init(STREAM_TARGET_DEV_ITER_AUTO, streams);
        const char* stream_name = Stream_target_dev_iter_get_next(iter);
        while (stream_name != NULL)
        {
            Voice_state* vstate = get_target_stream_vstate(ch, stream_name);
            if (vstate != NULL)
            {
                const Linear_controls* cs_controls =
                    Channel_stream_state_get_controls(stream_state, stream_name);
                if (Channel_stream_state_is_carrying_enabled(stream_state, stream_name))
                {
                    if (!isnan(Linear_controls_get_value(cs_controls)))
                        Stream_vstate_set_controls(vstate, cs_controls);
                }
                else
                {
                    Linear_controls new_lc;
                    Linear_controls_copy(&new_lc, Stream_vstate_get_controls(vstate));
                    Channel_stream_state_apply_overrides(
                            stream_state, stream_name, &new_lc);

                    Channel_stream_state_set_controls(
                            stream_state, stream_name, &new_lc);
                    Stream_vstate_set_controls(vstate, &new_lc);
                }
            }

            stream_name = Stream_target_dev_iter_get_next(iter);
        }
    }

    return;
}


bool Event_channel_note_on_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(ch->audio_rate > 0);
    rassert(ch->tempo > 0);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    // Move the old Voices to the background
    Event_channel_note_off_process(ch, dstates, master_params, NULL);

    ch->fg_count = 0;

    // Find our audio unit
    Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if (au == NULL)
        return true;

    // Allocate new Voices
    bool is_voice_rand_seed_set = false;
    uint64_t voice_rand_seed = 0;

    const uint64_t new_group_id = Voice_pool_new_group_id(ch->pool);

    double pitch_param = params->arg->value.float_type;

    // Retune pitch parameter if a retuner is active
    {
        const int tuning_index = master_params->cur_tuning_state;
        if (0 <= tuning_index && tuning_index < KQT_TUNING_TABLES_MAX)
        {
            Tuning_state* state = master_params->tuning_states[tuning_index];
            const Tuning_table* table =
                Module_get_tuning_table(master_params->parent.module, tuning_index);
            if (state != NULL && table != NULL)
                pitch_param = Tuning_state_get_retuned_pitch(state, table, pitch_param);
        }
    }

    if (ch->carry_pitch)
    {
        if (isnan(ch->pitch_controls.pitch))
            ch->pitch_controls.pitch = pitch_param;
        if (isnan(ch->pitch_controls.orig_carried_pitch))
            ch->pitch_controls.orig_carried_pitch = pitch_param;

        const double pitch_diff = pitch_param - ch->pitch_controls.orig_carried_pitch;
        ch->pitch_controls.pitch_add = pitch_diff;
    }
    else
    {
        Pitch_controls_reset(&ch->pitch_controls);
        ch->pitch_controls.orig_carried_pitch = pitch_param;

        Slider_set_tempo(&ch->pitch_controls.slider, master_params->tempo);
        Slider_set_length(&ch->pitch_controls.slider, &ch->pitch_slide_length);

        LFO_set_tempo(&ch->pitch_controls.vibrato, master_params->tempo);
        LFO_set_speed_slide(&ch->pitch_controls.vibrato, &ch->vibrato_speed_slide);
        LFO_set_depth_slide(&ch->pitch_controls.vibrato, &ch->vibrato_depth_slide);

        ch->pitch_controls.pitch = pitch_param;
        if (isnan(ch->pitch_controls.orig_carried_pitch))
            ch->pitch_controls.orig_carried_pitch = pitch_param;
    }

    init_force_controls(ch, master_params);

    // Don't attempt to play effects
    if (Audio_unit_get_type(au) != AU_TYPE_INSTRUMENT)
        return true;

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
            voice_rand_seed = Random_get_uint64(&ch->rand);
            is_voice_rand_seed_set = true;
        }

        reserve_voice(
                ch, au, new_group_id, proc_state, i, voice_rand_seed, params->external);

        Voice* voice = ch->fg[i];
        Voice_state* vs = voice->state;

        if (vs->proc_type == Proc_type_pitch)
            Pitch_vstate_set_controls(vs, &ch->pitch_controls);

        if (vs->proc_type == Proc_type_force)
        {
            Force_controls* fc = Force_vstate_get_force_controls_mut(vs);
            Force_controls_copy(fc, &ch->force_controls);
        }
    }

    Channel_reset_test_output(ch);

    Device_init_control_vars(
            (const Device*)au,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            Channel_get_random_source(ch),
            ch);

    init_streams(ch, au);

    return true;
}


bool Event_channel_hit_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(ch->audio_rate > 0);
    rassert(ch->tempo > 0);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_INT);

    // Move the old Voices to the background
    Event_channel_note_off_process(ch, dstates, master_params, NULL);

    ch->fg_count = 0;

    // Find our audio unit
    Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if (au == NULL)
        return true;

    bool is_voice_rand_seed_set = false;
    uint64_t voice_rand_seed = 0;

    const uint64_t new_group_id = Voice_pool_new_group_id(ch->pool);

    init_force_controls(ch, master_params);

    // Don't attempt to hit effects
    if (Audio_unit_get_type(au) != AU_TYPE_INSTRUMENT)
        return true;

    const int hit_index = (int)params->arg->value.int_type;

    if (!Audio_unit_get_hit_existence(au, hit_index))
        return true;

    const Param_proc_filter* hpf = Audio_unit_get_hit_proc_filter(au, hit_index);

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

        if (!is_voice_rand_seed_set)
        {
            voice_rand_seed = Random_get_uint64(&ch->rand);
            is_voice_rand_seed_set = true;
        }

        reserve_voice(
                ch, au, new_group_id, proc_state, i, voice_rand_seed, params->external);

        Voice* voice = ch->fg[i];
        Voice_state* vs = voice->state;
        vs->hit_index = hit_index;

        if (vs->proc_type == Proc_type_force)
        {
            Force_controls* fc = Force_vstate_get_force_controls_mut(vs);
            Force_controls_copy(fc, &ch->force_controls);
        }
    }

    Channel_reset_test_output(ch);

    Device_init_control_vars(
            (const Device*)au,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            Channel_get_random_source(ch),
            ch);

    init_streams(ch, au);

    return true;
}


bool Event_channel_note_off_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(params);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        if (ch->fg[i] != NULL)
        {
            ch->fg[i] = Voice_pool_get_voice(ch->pool, ch->fg[i], ch->fg_id[i]);
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


