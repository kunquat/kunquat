

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

#include <Generator.h>

#include <xmemory.h>


Gen_type Generator_get_type(Generator* gen)
{
    assert(gen != NULL);
    return gen->type;
}


void Generator_process_note(Generator* gen,
                            Voice_state* state,
                            int note,
                            int mod,
                            int octave)
{
    assert(gen != NULL);
    assert(state != NULL);
    assert(note >= 0);
    assert(note < NOTE_TABLE_NOTES);
    assert(mod < NOTE_TABLE_NOTE_MODS);
    assert(octave >= NOTE_TABLE_OCTAVE_FIRST);
    assert(octave <= NOTE_TABLE_OCTAVE_LAST);
    if (gen->ins_params->notes == NULL || *gen->ins_params->notes == NULL)
    {
        return;
    }
    pitch_t freq = Note_table_get_pitch(*gen->ins_params->notes, note, mod, octave);
    if (freq > 0)
    {
        state->freq = freq;
    }
    return;
}


void Generator_mix(Generator* gen,
                   Voice_state* state,
                   uint32_t nframes,
                   uint32_t offset,
                   uint32_t freq)
{
    assert(gen != NULL);
    assert(gen->mix != NULL);
    uint32_t mixed = offset;
    while (mixed < nframes)
    {
        mixed = gen->mix(gen, state, nframes, mixed, freq,
                         gen->ins_params->buf_count,
                         gen->ins_params->bufs);
        if (!state->active)
        {
            break;
        }
    }
    return;
}


void del_Generator(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->destroy != NULL);
    gen->destroy(gen);
    return;
}


