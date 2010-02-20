

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <Event_common.h>
#include <Event_channel_note_on.h>
#include <Event_channel_note_off.h>
#include <Reltime.h>
#include <Voice.h>
#include <Scale.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc note_on_desc[] =
{
    {
        .type = EVENT_FIELD_NOTE,
        .min.field.integral_type = 0,
        .max.field.integral_type = KQT_SCALE_NOTES - 1
    },
    {
        .type = EVENT_FIELD_NOTE_MOD,
        .min.field.integral_type = -1,
        .max.field.integral_type = KQT_SCALE_NOTE_MODS - 1
    },
    {
        .type = EVENT_FIELD_INT,
        .min.field.integral_type = KQT_SCALE_OCTAVE_FIRST,
        .max.field.integral_type = KQT_SCALE_OCTAVE_LAST
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_channel_note_on_set(Event* event, int index, void* data);

static void* Event_channel_note_on_get(Event* event, int index);


Event_create_constructor(Event_channel_note_on,
                         EVENT_CHANNEL_NOTE_ON,
                         note_on_desc,
                         event->note = 0,
                         event->mod = -1,
                         event->octave = KQT_SCALE_MIDDLE_OCTAVE);


bool Event_channel_note_on_handle(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[3];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, note_on_desc, data, state);
    if (state->error)
    {
        return false;
    }
    // move the old Voices to the background
    Event_channel_note_off_handle(ch_state, NULL);
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
    ch_state->panning_slide = 0;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        if (Instrument_get_gen(ins, i) == NULL)
        {
            continue;
        }
        ++ch_state->fg_count;
        ch_state->fg[i] = Voice_pool_get_voice(ch_state->pool, NULL, 0);
        assert(ch_state->fg[i] != NULL);
//        fprintf(stderr, "allocated Voice %p\n", (void*)ch_state->fg[i]);
        ch_state->fg_id[i] = Voice_id(ch_state->fg[i]);
        Voice_init(ch_state->fg[i],
                   Instrument_get_gen(ins, i),
                   &ch_state->vp,
                   *ch_state->freq,
                   *ch_state->tempo);
        Voice_pool_fix_priority(ch_state->pool, ch_state->fg[i]);

        Voice* voice = ch_state->fg[i];
        Voice_state* vs = &voice->state.generic;
        Generator_process_note(voice->gen,
                               vs,
                               data[0].field.integral_type,
                               data[1].field.integral_type,
                               data[2].field.integral_type);
        vs->orig_note = data[0].field.integral_type;
        vs->orig_note_mod = data[1].field.integral_type;
        vs->orig_octave = data[2].field.integral_type;

        vs->pedal = &voice->gen->ins_params->pedal;

        Reltime_copy(&vs->force_slide_length, &ch_state->force_slide_length);
        vs->tremolo_length = ch_state->tremolo_length;
        vs->tremolo_update = ch_state->tremolo_update;
        vs->tremolo_depth_target = ch_state->tremolo_depth;
        vs->tremolo_delay_update = ch_state->tremolo_delay_update;

        Reltime_copy(&vs->pitch_slide_length, &ch_state->pitch_slide_length);
        vs->vibrato_length = ch_state->vibrato_length;
        vs->vibrato_update = ch_state->vibrato_update;
        vs->vibrato_depth_target = ch_state->vibrato_depth;
        vs->vibrato_delay_update = ch_state->vibrato_delay_update;

        vs->panning = ch_state->panning;
        vs->panning_slide = ch_state->panning_slide;
        Reltime_copy(&vs->panning_slide_length, &ch_state->panning_slide_length);
        vs->panning_slide_target = ch_state->panning_slide_target;
        vs->panning_slide_frames = ch_state->panning_slide_frames;
        vs->panning_slide_update = ch_state->panning_slide_update;

        Reltime_copy(&vs->filter_slide_length, &ch_state->filter_slide_length);
    }
    return true;
}


static bool Event_channel_note_on_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_CHANNEL_NOTE_ON);
    assert(data != NULL);
    Event_channel_note_on* note_on = (Event_channel_note_on*)event;
    int64_t num = *(int64_t*)data;
    if (index < 0 || index >= 3)
    {
        return false;
    }
    Event_check_integral_range(num, event->field_types[index]);
    switch (index)
    {
        case 0:
        {
            note_on->note = num;
        }
        break;
        case 1:
        {
            note_on->mod = num;
        }
        break;
        case 2:
        {
            note_on->octave = num;
        }
        break;
        default:
        {
            assert(false);
        }
        break;
    }
    return true;
}


static void* Event_channel_note_on_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_CHANNEL_NOTE_ON);
    Event_channel_note_on* note_on = (Event_channel_note_on*)event;
    switch (index)
    {
        case 0:
        {
            return &note_on->note;
        }
        break;
        case 1:
        {
            return &note_on->mod;
        }
        break;
        case 2:
        {
            return &note_on->octave;
        }
        break;
        default:
        break;
    }
    return NULL;
}


