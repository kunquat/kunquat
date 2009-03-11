

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "Instrument.h"
#include "Instrument_debug.h"
#include "Instrument_sine.h"
#include "Instrument_pcm.h"

#include <xmemory.h>


Instrument* new_Instrument(Ins_type type,
        frame_t** bufs,
        uint32_t buf_len,
        uint8_t events)
{
    assert(type > INS_TYPE_NONE);
    assert(type < INS_TYPE_LAST);
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    assert(bufs[1] != NULL);
    assert(buf_len > 0);
    assert(events > 0);
    Instrument* ins = xalloc(Instrument);
    if (ins == NULL)
    {
        return NULL;
    }
    ins->pbufs = NULL;
    ins->events = NULL;
    ins->notes = NULL;
    ins->force_volume_env = NULL;
    ins->force_filter_env = NULL;
    ins->force_pitch_env = NULL;
    ins->volume_env = NULL;
    ins->volume_off_env = NULL;
    ins->pitch_pan_env = NULL;
    ins->filter_env = NULL;
    ins->filter_off_env = NULL;
    ins->type_data = NULL;
    ins->init = NULL;
    ins->init_state = NULL;
    ins->uninit = NULL;
    ins->mix = NULL;
    ins->events = new_Event_queue(events);
    if (ins->events == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }

    ins->pedal = false;

    ins->default_force = 1;
    ins->force_variation = 0;

    ins->force_volume_env = new_Envelope(8,
            0, 1, 0, // min, max, step of x
            0, 1, 0); // min, max, step of y
    if (ins->force_volume_env == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }
    ins->force_volume_env_enabled = false;
    Envelope_set_node(ins->force_volume_env, 0, 0);
    Envelope_set_node(ins->force_volume_env, 1, 1);
    Envelope_set_first_lock(ins->force_volume_env, true, true);
    Envelope_set_last_lock(ins->force_volume_env, true, false);

    ins->force_filter_env = new_Envelope(8,
            0, 1, 0, // min, max, step of x
            0, 1, 0); // min, max, step of y
    if (ins->force_filter_env == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }
    ins->force_filter_env_enabled = false;
    Envelope_set_node(ins->force_filter_env, 0, 1);
    Envelope_set_node(ins->force_filter_env, 1, 1);
    Envelope_set_first_lock(ins->force_filter_env, true, false);
    Envelope_set_last_lock(ins->force_filter_env, true, false);

    ins->force_pitch_env = new_Envelope(8,
            0, 1, 0, // min, max, step of x
            -1, 1, 0); // min, max, step of y
    if (ins->force_pitch_env == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }
    ins->force_pitch_env_enabled = false;
    Envelope_set_node(ins->force_pitch_env, 0, 0);
    Envelope_set_node(ins->force_pitch_env, 1, 0);
    Envelope_set_first_lock(ins->force_pitch_env, true, false);
    Envelope_set_last_lock(ins->force_pitch_env, true, false);

    ins->volume = 1;
    ins->volume_env = new_Envelope(32,
            0, INFINITY, 0, // min, max, step of x
            0, 1, 0); // min, max, step of y
    if (ins->volume_env == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }
    ins->volume_env_enabled = false;
    ins->volume_env_carry = false;
    ins->volume_env_scale = 1;
    ins->volume_env_center = 440;
    Envelope_set_node(ins->volume_env, 0, 1);
    Envelope_set_node(ins->volume_env, 1, 1);
    Envelope_set_first_lock(ins->volume_env, true, false);

    ins->volume_off_env = new_Envelope(32,
            0, INFINITY, 0, // min, max, step of x
            0, 1, 0); // min, max, step of y
    if (ins->volume_off_env == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }
    ins->volume_off_env_enabled = true;
    ins->volume_off_env_scale = 1;
    ins->volume_off_env_center = 440;
    Envelope_set_node(ins->volume_off_env, 0, 1);
    Envelope_set_node(ins->volume_off_env, 0.2, 0.4);
    Envelope_set_node(ins->volume_off_env, 2, 0);
    Envelope_set_first_lock(ins->volume_off_env, true, false);
    Envelope_set_last_lock(ins->volume_off_env, false, true);

    ins->pitch_pan_env = new_Envelope(8,
            -1, 1, 0, // min, max, step of x
            -1, 1, 0); // min, max, step of y
    if (ins->pitch_pan_env == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }
    ins->pitch_pan_env_enabled = false;
    Envelope_set_node(ins->pitch_pan_env, -1, 0);
    Envelope_set_node(ins->pitch_pan_env, 0, 0);
    Envelope_set_node(ins->pitch_pan_env, 1, 0);
    Envelope_set_first_lock(ins->pitch_pan_env, true, false);
    Envelope_set_last_lock(ins->pitch_pan_env, true, false);

    ins->filter_env = new_Envelope(32,
            0, INFINITY, 0, // min, max, step of x
            0, 1, 0); // min, max, step of y
    if (ins->filter_env == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }
    ins->filter_env_enabled = false;
    ins->filter_env_scale = 1;
    ins->filter_env_center = 440;
    Envelope_set_node(ins->filter_env, 0, 1);
    Envelope_set_node(ins->filter_env, 1, 1);
    Envelope_set_first_lock(ins->filter_env, true, false);

    ins->filter_off_env = new_Envelope(32,
            0, INFINITY, 0, // min, max, step of x
            0, 1, 0); // min, max, step of y
    if (ins->filter_off_env == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }
    ins->filter_off_env_enabled = false;
    ins->filter_off_env_scale = 1;
    ins->filter_off_env_center = 440;
    Envelope_set_node(ins->filter_off_env, 0, 1);
    Envelope_set_node(ins->filter_off_env, 1, 1);
    Envelope_set_first_lock(ins->filter_off_env, true, false);

    ins->type = type;
    ins->name[0] = ins->name[INS_NAME_MAX - 1] = L'\0';
    ins->bufs = ins->gbufs = bufs;
    ins->buf_len = buf_len;
    switch (type)
    {
        case INS_TYPE_DEBUG:
            ins->mix = Instrument_debug_mix;
            break;
        case INS_TYPE_SINE:
            ins->mix = Instrument_sine_mix;
            ins->init_state = Voice_state_sine_init;
            break;
        case INS_TYPE_PCM:
            ins->mix = Instrument_pcm_mix;
            ins->init = Instrument_pcm_init;
            ins->uninit = Instrument_pcm_uninit;
            break;
        default:
            ins->mix = NULL;
            assert(false);
    }
    assert((ins->init == NULL) == (ins->uninit == NULL));
    if (ins->init != NULL)
    {
        if (ins->init(ins) != 0)
        {
            del_Instrument(ins);
            return NULL;
        }
    }
    return ins;
}


Ins_type Instrument_get_type(Instrument* ins)
{
    assert(ins != NULL);
    return ins->type;
}


void Instrument_set_name(Instrument* ins, wchar_t* name)
{
    assert(ins != NULL);
    assert(name != NULL);
    wcsncpy(ins->name, name, INS_NAME_MAX - 1);
    ins->name[INS_NAME_MAX - 1] = L'\0';
    return;
}


wchar_t* Instrument_get_name(Instrument* ins)
{
    assert(ins != NULL);
    return ins->name;
}


void Instrument_set_note_table(Instrument* ins, Note_table** notes)
{
    assert(ins != NULL);
    assert(notes != NULL);
    ins->notes = notes;
    return;
}


void Instrument_process_note(Instrument* ins,
        Voice_state* state,
        int note,
        int mod,
        int octave)
{
    assert(ins != NULL);
    assert(state != NULL);
    assert(note >= 0);
    assert(note < NOTE_TABLE_NOTES);
    assert(mod < NOTE_TABLE_NOTE_MODS);
    assert(octave >= NOTE_TABLE_OCTAVE_FIRST);
    assert(octave <= NOTE_TABLE_OCTAVE_LAST);
    if (ins->notes == NULL || *ins->notes == NULL)
    {
        return;
    }
    pitch_t freq = Note_table_get_pitch(*ins->notes, note, mod, octave);
    if (freq > 0)
    {
        state->freq = freq;
    }
    return;
}


void Instrument_mix(Instrument* ins,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(ins != NULL);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len);
    assert(freq > 0);
    assert(ins->mix != NULL);
    if (!state->active)
    {
        return;
    }
    if (ins->mix != NULL)
    {
        ins->mix(ins, state, nframes, offset, freq);
    }
    return;
}


void del_Instrument(Instrument* ins)
{
    assert(ins != NULL);
    if (ins->force_volume_env != NULL)
    {
        del_Envelope(ins->force_volume_env);
    }
    if (ins->force_filter_env != NULL)
    {
        del_Envelope(ins->force_filter_env);
    }
    if (ins->force_pitch_env != NULL)
    {
        del_Envelope(ins->force_pitch_env);
    }
    if (ins->volume_env != NULL)
    {
        del_Envelope(ins->volume_env);
    }
    if (ins->volume_off_env != NULL)
    {
        del_Envelope(ins->volume_off_env);
    }
    if (ins->pitch_pan_env != NULL)
    {
        del_Envelope(ins->pitch_pan_env);
    }
    if (ins->filter_env != NULL)
    {
        del_Envelope(ins->filter_env);
    }
    if (ins->filter_off_env != NULL)
    {
        del_Envelope(ins->filter_off_env);
    }
    if (ins->events != NULL)
    {
        del_Event_queue(ins->events);
    }
    if (ins->uninit != NULL)
    {
        ins->uninit(ins);
    }
    if (ins->pbufs != NULL)
    {
        assert(ins->pbufs[0] != NULL);
        assert(ins->pbufs[1] != NULL);
        xfree(ins->pbufs[0]);
        xfree(ins->pbufs[1]);
        xfree(ins->pbufs);
    }
    xfree(ins);
}


