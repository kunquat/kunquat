

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <Event_channel_decl.h>
#include <Event_common.h>
#include <Value.h>
#include <Voice.h>
#include <xassert.h>


bool Event_channel_arpeggio_on_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    (void)value;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice* voice = ch_state->fg[i];
        Voice_state* vs = voice->state;
        //pitch_t orig_pitch = -1;
        if (vs->arpeggio || voice->gen->ins_params->pitch_locks[i].enabled)
        {
            continue;
        }
#if 0
        if (voice->gen->ins_params->scale != NULL &&
                *voice->gen->ins_params->scale != NULL &&
                **voice->gen->ins_params->scale != NULL)
        {
            orig_pitch = Scale_get_pitch_from_cents(
                         **voice->gen->ins_params->scale, vs->orig_cents);
        }
        else
        {
            orig_pitch = exp2(vs->orig_cents / 1200) * 440;
        }
        if (orig_pitch <= 0)
        {
            vs->arpeggio = false;
            continue;
        }
#endif
        if (isnan(ch_state->arpeggio_ref))
        {
            ch_state->arpeggio_ref = vs->orig_cents;
        }
        if (isnan(ch_state->arpeggio_tones[0]))
        {
            ch_state->arpeggio_tones[0] = ch_state->arpeggio_ref;
        }
        vs->arpeggio_ref = ch_state->arpeggio_ref;
        memcpy(vs->arpeggio_tones, ch_state->arpeggio_tones,
                KQT_ARPEGGIO_NOTES_MAX * sizeof(double));
#if 0
        int last_nonzero = -1;
        for (int k = 0; k < KQT_ARPEGGIO_NOTES_MAX; ++k)
        {
            if (data[k + 1].field.double_type != 0)
            {
                last_nonzero = k;
            }
            pitch_t new_pitch = -1;
            if (voice->gen->ins_params->scale != NULL &&
                    *voice->gen->ins_params->scale != NULL &&
                    **voice->gen->ins_params->scale != NULL)
            {
                Scale* scale = **voice->gen->ins_params->scale;
                new_pitch = Scale_get_pitch_from_cents(scale,
                            vs->orig_cents + data[k + 1].field.double_type);
            }
            else
            {
                new_pitch = vs->orig_cents + data[k + 1].field.double_type;
            }
            if (new_pitch <= 0)
            {
                last_nonzero = -1;
                break;
            }
            else
            {
                vs->arpeggio_factors[k] = new_pitch / orig_pitch;
            }
        }
        if (last_nonzero == -1)
        {
            vs->arpeggio = false;
            continue;
        }
        else if (last_nonzero < KQT_ARPEGGIO_NOTES_MAX - 1)
        {
            vs->arpeggio_factors[last_nonzero + 1] = -1;
        }
#endif
        double unit_len = Tstamp_toframes(
                Tstamp_set(TSTAMP_AUTO, 1, 0),
                *ch_state->tempo,
                *ch_state->freq);
        vs->arpeggio_length = unit_len / ch_state->arpeggio_speed;
        vs->arpeggio_frames = 0;
        vs->arpeggio_note = 0;
        vs->arpeggio = true;
    }
    return true;
}


bool Event_channel_arpeggio_off_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    (void)value;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        ch_state->fg[i]->state->arpeggio = false;
    }
    return true;
}


bool Event_channel_set_arpeggio_index_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    ch_state->arpeggio_edit_pos = value->value.int_type;
    return true;
}


bool Event_channel_set_arpeggio_note_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    if (isnan(ch_state->arpeggio_tones[ch_state->arpeggio_edit_pos]) &&
            ch_state->arpeggio_edit_pos < KQT_ARPEGGIO_NOTES_MAX - 1)
    {
        ch_state->arpeggio_tones[ch_state->arpeggio_edit_pos + 1] = NAN;
        for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
        {
            Event_check_voice(ch_state, i);
            Voice_state* vs = ch_state->fg[i]->state;
            vs->arpeggio_tones[ch_state->arpeggio_edit_pos + 1] = NAN;
        }
    }
    ch_state->arpeggio_tones[ch_state->arpeggio_edit_pos] =
            value->value.float_type;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->arpeggio_tones[ch_state->arpeggio_edit_pos] =
                value->value.float_type;
    }
    if (ch_state->arpeggio_edit_pos < KQT_ARPEGGIO_NOTES_MAX - 1)
    {
        ++ch_state->arpeggio_edit_pos;
    }
    return true;
}


bool Event_channel_set_arpeggio_speed_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    ch_state->arpeggio_speed = value->value.float_type;
    double unit_len = Tstamp_toframes(
            Tstamp_set(TSTAMP_AUTO, 1, 0),
            *ch_state->tempo,
            *ch_state->freq);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->arpeggio_length = unit_len / value->value.float_type;
    }
    return true;
}


bool Event_channel_reset_arpeggio_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    (void)value;
    ch_state->arpeggio_ref = NAN;
    ch_state->arpeggio_edit_pos = 1;
    ch_state->arpeggio_tones[0] = ch_state->arpeggio_tones[1] = NAN;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->arpeggio_tones[0] = vs->arpeggio_tones[1] = NAN;
        vs->arpeggio_note = 0;
    }
    return true;
}


