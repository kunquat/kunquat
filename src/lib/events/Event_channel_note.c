

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <stdio.h>
#include <float.h>

#include <Event_channel_decl.h>
#include <Event_common.h>
#include <kunquat/limits.h>
#include <note_setup.h>
#include <Random.h>
#include <Tstamp.h>
#include <Scale.h>
#include <Voice.h>
#include <Value.h>
#include <xassert.h>


bool Event_channel_note_on_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    assert(ch_state->freq != NULL);
    assert(*ch_state->freq > 0);
    assert(ch_state->tempo != NULL);
    assert(*ch_state->tempo > 0);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    // move the old Voices to the background
    Event_channel_note_off_process(ch_state, NULL);
    ch_state->fg_count = 0;
    assert(ch_state->instrument >= 0);
    assert(ch_state->instrument < KQT_INSTRUMENTS_MAX);
    Instrument* ins = Ins_table_get(ch_state->insts,
                                    ch_state->instrument);
    if (ins == NULL)
    {
        return true;
    }
    // allocate new Voices
//    ch_state->panning_slide = 0;
    double force_var = NAN;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        const Generator* gen = Instrument_get_gen(ins, i);
        if (gen == NULL || !Device_is_existent((const Device*)gen))
        {
            continue;
        }
        reserve_voice(ch_state, ins, i);

        Voice* voice = ch_state->fg[i];
        Voice_state* vs = voice->state;

        if (voice->gen->ins_params->pitch_locks[i].enabled)
        {
            vs->pitch = voice->gen->ins_params->pitch_locks[i].freq;
        }
#if 0
        else if (voice->gen->ins_params->scale == NULL ||
                 *voice->gen->ins_params->scale == NULL ||
                 **voice->gen->ins_params->scale == NULL)
        {
            vs->pitch = exp2(value->value.float_type / 1200) * 440;
        }
        else
#endif
        {
#if 0
            pitch_t pitch = Scale_get_pitch_from_cents(
                    **voice->gen->ins_params->scale,
                    value->value.float_type);
            if (pitch > 0)
            {
                vs->pitch = pitch;
            }
            else
#endif
            {
                vs->pitch = exp2(value->value.float_type / 1200) * 440;
            }
        }
        //fprintf(stderr, "Event set pitch @ %p: %f\n", (void*)&vs->pitch, vs->pitch);
        vs->orig_cents = value->value.float_type;

        set_instrument_properties(voice, vs, ch_state, &force_var);
    }
    return true;
}


bool Event_channel_hit_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    assert(ch_state->freq != NULL);
    assert(ch_state->tempo != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    // move the old Voices to the background
    Event_channel_note_off_process(ch_state, NULL);
    ch_state->fg_count = 0;
    assert(ch_state->instrument >= 0);
    assert(ch_state->instrument < KQT_INSTRUMENTS_MAX);
    Instrument* ins = Ins_table_get(
            ch_state->insts,
            ch_state->instrument);
    if (ins == NULL)
    {
        return true;
    }
    double force_var = NAN;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        const Generator* gen = Instrument_get_gen(ins, i);
        if (gen == NULL || !Device_is_existent((const Device*)gen))
        {
            continue;
        }
        reserve_voice(ch_state, ins, i);
        Voice* voice = ch_state->fg[i];
        Voice_state* vs = voice->state;
        vs->hit_index = value->value.int_type;
        set_instrument_properties(voice, vs, ch_state, &force_var);
    }
    return true;
}


bool Event_channel_note_off_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    (void)value;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        if (ch_state->fg[i] != NULL)
        {
            ch_state->fg[i] = Voice_pool_get_voice(
                    ch_state->pool,
                    ch_state->fg[i],
                    ch_state->fg_id[i]);
            if (ch_state->fg[i] == NULL)
            {
                // The Voice has been given to another channel
                continue;
            }
            ch_state->fg[i]->state->note_on = false;
            ch_state->fg[i]->prio = VOICE_PRIO_BG;
            Voice_pool_fix_priority(ch_state->pool, ch_state->fg[i]);
            ch_state->fg[i] = NULL;
        }
    }
    ch_state->fg_count = 0;
    return true;
}


