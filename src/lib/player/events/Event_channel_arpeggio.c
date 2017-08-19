

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


#include <player/events/Event_channel_decl.h>

#include <debug/assert.h>
#include <init/devices/Proc_type.h>
#include <player/Channel.h>
#include <player/devices/processors/Pitch_state.h>
#include <player/devices/Voice_state.h>
#include <player/events/Event_common.h>
#include <player/Voice.h>
#include <player/Voice_pool.h>
#include <Value.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


bool Event_channel_arpeggio_on_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(value);

    if (isnan(ch->arpeggio_ref))
        ch->arpeggio_ref = ch->pitch_controls.orig_carried_pitch;

    if (isnan(ch->arpeggio_tones[0]))
        ch->arpeggio_tones[0] = ch->arpeggio_ref;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice* voice = ch->fg[i];
        Voice_state* vs = voice->state;

        if (vs->proc_type == Proc_type_pitch)
            Pitch_vstate_arpeggio_on(
                    vs, ch->arpeggio_speed, ch->arpeggio_ref, ch->arpeggio_tones);
    }

    return true;
}


bool Event_channel_arpeggio_off_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(value);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (vs->proc_type == Proc_type_pitch)
            Pitch_vstate_arpeggio_off(vs);
    }

    return true;
}


bool Event_channel_set_arpeggio_index_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_INT);

    ch->arpeggio_edit_pos = (int)value->value.int_type;

    return true;
}


bool Event_channel_set_arpeggio_note_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    if (isnan(ch->arpeggio_tones[ch->arpeggio_edit_pos]) &&
            ch->arpeggio_edit_pos < KQT_ARPEGGIO_TONES_MAX - 1)
        ch->arpeggio_tones[ch->arpeggio_edit_pos + 1] = NAN;

    ch->arpeggio_tones[ch->arpeggio_edit_pos] = value->value.float_type;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (vs->proc_type == Proc_type_pitch)
            Pitch_vstate_update_arpeggio_tones(vs, ch->arpeggio_tones);
    }

    if (ch->arpeggio_edit_pos < KQT_ARPEGGIO_TONES_MAX - 1)
        ++ch->arpeggio_edit_pos;

    return true;
}


bool Event_channel_set_arpeggio_speed_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    ch->arpeggio_speed = value->value.float_type;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (vs->proc_type == Proc_type_pitch)
            Pitch_vstate_update_arpeggio_speed(vs, ch->arpeggio_speed);
    }

    return true;
}


bool Event_channel_reset_arpeggio_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(value);

    ch->arpeggio_ref = NAN;
    ch->arpeggio_edit_pos = 1;
    ch->arpeggio_tones[0] = ch->arpeggio_tones[1] = NAN;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (vs->proc_type == Proc_type_pitch)
            Pitch_vstate_reset_arpeggio(vs);
    }

    return true;
}


