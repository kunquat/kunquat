

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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

#include <Input_map.h>
#include <kunquat/limits.h>
#include <Module.h>
#include <player/events/Event_channel_decl.h>
#include <player/events/Event_common.h>
#include <player/events/note_setup.h>
#include <player/Voice.h>
#include <Random.h>
#include <Scale.h>
#include <Tstamp.h>
#include <Value.h>
#include <xassert.h>


bool Event_channel_note_on_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
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

    // Find our instrument
    Instrument* ins = Module_get_ins_from_input(
            ch->parent.module,
            ch->ins_input);
    if (ins == NULL)
        return true;

    // Allocate new Voices
//    ch->panning_slide = 0;
    double force_var = NAN;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        const Generator* gen = Instrument_get_gen(ins, i);
        if (gen == NULL || !Device_is_existent((const Device*)gen))
            continue;

        const Gen_state* gen_state = (Gen_state*)Device_states_get_state(
                dstates,
                Device_get_id((Device*)gen));

        reserve_voice(ch, ins, gen_state, i);

        Voice* voice = ch->fg[i];
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

        set_instrument_properties(voice, vs, ch, &force_var);
    }

    return true;
}


bool Event_channel_hit_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
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

    // Find our instrument
    Instrument* ins = Module_get_ins_from_input(
            ch->parent.module,
            ch->ins_input);
    if (ins == NULL)
        return true;

    double force_var = NAN;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        const Generator* gen = Instrument_get_gen(ins, i);
        if (gen == NULL || !Device_is_existent((const Device*)gen))
            continue;

        const Gen_state* gen_state = (Gen_state*)Device_states_get_state(
                dstates,
                Device_get_id((Device*)gen));

        reserve_voice(ch, ins, gen_state, i);

        Voice* voice = ch->fg[i];
        Voice_state* vs = voice->state;
        vs->hit_index = value->value.int_type;
        set_instrument_properties(voice, vs, ch, &force_var);
    }

    return true;
}


bool Event_channel_note_off_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    (void)value;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
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


