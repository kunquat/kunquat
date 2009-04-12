

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

#include <Generator.h>
#include "Instrument.h"

#include <xmemory.h>


Instrument* new_Instrument(frame_t** bufs,
        uint32_t buf_len,
        uint8_t events)
{
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
    if (Instrument_params_init(&ins->params, bufs, buf_len) == NULL)
    {
        xfree(ins);
        return NULL;
    }
    ins->events = NULL;
    ins->notes = NULL;
    ins->events = new_Event_queue(events);
    if (ins->events == NULL)
    {
        del_Instrument(ins);
        return NULL;
    }

    ins->default_force = 1;
    ins->force_variation = 0;

    for (int i = 0; i < GENERATORS_MAX; ++i)
    {
        ins->gens[i] = NULL;
    }

    ins->name[0] = ins->name[INS_NAME_MAX - 1] = L'\0';
    return ins;
}


Instrument_params* Instrument_get_params(Instrument* ins)
{
    assert(ins != NULL);
    return &ins->params;
}


int Instrument_set_gen(Instrument* ins,
        int index,
        Generator* gen)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < GENERATORS_MAX);
    assert(gen != NULL);
    if (ins->gens[index] != NULL)
    {
        del_Generator(ins->gens[index]);
        ins->gens[index] = NULL;
    }
    while (index > 0 && ins->gens[index - 1] == NULL)
    {
        --index;
    }
    ins->gens[index] = gen;
    return index;
}


Generator* Instrument_get_gen(Instrument* ins,
        int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < GENERATORS_MAX);
    return ins->gens[index];
}


void Instrument_del_gen(Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < GENERATORS_MAX);
    if (ins->gens[index] == NULL)
    {
        return;
    }
    del_Generator(ins->gens[index]);
    ins->gens[index] = NULL;
    while (index < GENERATORS_MAX - 1 && ins->gens[index + 1] != NULL)
    {
        ins->gens[index] = ins->gens[index + 1];
        ins->gens[index + 1] = NULL;
        ++index;
    }
    return;
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
        Voice_state* states,
        int note,
        int mod,
        int octave)
{
    assert(ins != NULL);
    assert(states != NULL);
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
        for (int i = 0; i < GENERATORS_MAX && ins->gens[i] != NULL; ++i)
        {
            states[i].freq = freq;
        }
    }
    return;
}


void Instrument_mix(Instrument* ins,
        Voice_state* states,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(ins != NULL);
    assert(states != NULL);
//  assert(nframes <= ins->buf_len);
    assert(freq > 0);
    for (int i = 0; i < GENERATORS_MAX && ins->gens[i] != NULL; ++i)
    {
        Generator_mix(ins->gens[i], &states[i], nframes, offset, freq);
    }
    return;
}


void del_Instrument(Instrument* ins)
{
    assert(ins != NULL);
    Instrument_params_uninit(&ins->params);
    if (ins->events != NULL)
    {
        del_Event_queue(ins->events);
    }
    for (int i = 0; i < GENERATORS_MAX && ins->gens[i] != NULL; ++i)
    {
        del_Generator(ins->gens[i]);
    }
    xfree(ins);
}


