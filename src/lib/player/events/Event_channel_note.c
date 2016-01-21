

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
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
#include <init/Input_map.h>
#include <init/Module.h>
#include <init/Scale.h>
#include <kunquat/limits.h>
#include <mathnum/conversions.h>
#include <mathnum/Tstamp.h>
#include <player/Channel.h>
#include <player/devices/processors/Force_state.h>
#include <player/devices/processors/Pitch_state.h>
#include <player/devices/processors/Stream_state.h>
#include <player/devices/Voice_state.h>
#include <player/events/Event_common.h>
#include <player/events/note_setup.h>
#include <player/events/stream_utils.h>
#include <player/Voice.h>
#include <Value.h>

#include <float.h>
#include <math.h>
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
    bool is_voice_rand_seed_set = false;
    uint64_t voice_rand_seed = 0;

    const uint64_t new_group_id = Voice_pool_new_group_id(ch->pool);

    if (ch->carry_pitch)
    {
        const double cents = value->value.float_type;

        if (isnan(ch->pitch_controls.pitch))
            ch->pitch_controls.pitch = cents;
        if (isnan(ch->pitch_controls.orig_carried_pitch))
            ch->pitch_controls.orig_carried_pitch = cents;

        const double pitch_diff = cents - ch->pitch_controls.orig_carried_pitch;
        ch->pitch_controls.pitch_add = pitch_diff;
    }
    else
    {
        Pitch_controls_reset(&ch->pitch_controls);
        ch->pitch_controls.orig_carried_pitch = value->value.float_type;
        Slider_set_length(&ch->pitch_controls.slider, &ch->pitch_slide_length);
        LFO_set_speed_slide(&ch->pitch_controls.vibrato, &ch->vibrato_speed_slide);
        LFO_set_depth_slide(&ch->pitch_controls.vibrato, &ch->vibrato_depth_slide);

        ch->pitch_controls.pitch = value->value.float_type;
        if (isnan(ch->pitch_controls.orig_carried_pitch))
            ch->pitch_controls.orig_carried_pitch = value->value.float_type;
    }

    if (!ch->carry_force)
    {
        Force_controls_reset(&ch->force_controls);
        ch->force_controls.force = 1;
        Slider_set_length(&ch->force_controls.slider, &ch->force_slide_length);
        LFO_set_speed_slide(&ch->force_controls.tremolo, &ch->tremolo_speed_slide);
        LFO_set_depth_slide(&ch->force_controls.tremolo, &ch->tremolo_depth_slide);
    }

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

        if (vs->is_pitch_state)
            Pitch_vstate_set_controls(vs, &ch->pitch_controls);

        if (vs->is_force_state)
        {
            Force_controls* fc = Force_vstate_get_force_controls_mut(vs);
            Force_controls_copy(fc, &ch->force_controls);
        }
    }

    Device_init_control_vars(
            (const Device*)au,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            Channel_get_random_source(ch),
            ch);

    // Initialise streams
    const Au_streams* streams = Audio_unit_get_streams(au);
    if (streams != NULL)
    {
        const Channel_stream_state* stream_state = Channel_get_stream_state(ch);

        Stream_target_dev_iter* iter =
            Stream_target_dev_iter_init(STREAM_TARGET_DEV_ITER_AUTO, streams);
        const char* stream_name = Stream_target_dev_iter_get_next(iter);
        while (stream_name != NULL)
        {
            if (Channel_stream_state_is_carrying_enabled(stream_state, stream_name))
            {
                Voice_state* vstate = get_target_stream_vstate(ch, stream_name);
                if (vstate != NULL)
                {
                    const Linear_controls* controls =
                        Channel_stream_state_get_controls(stream_state, stream_name);
                    if (!isnan(Linear_controls_get_value(controls)))
                        Stream_vstate_set_controls(vstate, controls);
                }
            }

            stream_name = Stream_target_dev_iter_get_next(iter);
        }
    }

    return true;
}


bool Event_channel_hit_process(Channel* ch, Device_states* dstates, const Value* value)
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
    Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if (au == NULL)
        return true;

    bool is_voice_rand_seed_set = false;
    uint64_t voice_rand_seed = 0;

    const uint64_t new_group_id = Voice_pool_new_group_id(ch->pool);

    if (!ch->carry_force)
    {
        Force_controls_reset(&ch->force_controls);
        ch->force_controls.force = 1;
        Slider_set_length(&ch->force_controls.slider, &ch->force_slide_length);
        LFO_set_speed_slide(&ch->force_controls.tremolo, &ch->tremolo_speed_slide);
        LFO_set_depth_slide(&ch->force_controls.tremolo, &ch->tremolo_depth_slide);
    }

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

        if (vs->is_force_state)
        {
            Force_controls* fc = Force_vstate_get_force_controls_mut(vs);
            Force_controls_copy(fc, &ch->force_controls);
        }
    }

    Device_init_control_vars(
            (const Device*)au,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            Channel_get_random_source(ch),
            ch);

    // Initialise streams
    {
        const Au_streams* streams = Audio_unit_get_streams(au);

        const Channel_stream_state* stream_state = Channel_get_stream_state(ch);

        Stream_target_dev_iter* iter =
            Stream_target_dev_iter_init(STREAM_TARGET_DEV_ITER_AUTO, streams);
        const char* stream_name = Stream_target_dev_iter_get_next(iter);
        while (stream_name != NULL)
        {
            if (Channel_stream_state_is_carrying_enabled(stream_state, stream_name))
            {
                Voice_state* vstate = get_target_stream_vstate(ch, stream_name);
                if (vstate != NULL)
                {
                    const Linear_controls* controls =
                        Channel_stream_state_get_controls(stream_state, stream_name);
                    if (!isnan(Linear_controls_get_value(controls)))
                        Stream_vstate_set_controls(vstate, controls);
                }
            }

            stream_name = Stream_target_dev_iter_get_next(iter);
        }
    }

    return true;
}


bool Event_channel_note_off_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    ignore(value);

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


