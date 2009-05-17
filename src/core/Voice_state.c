

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

#include <Voice_state.h>


Voice_state* Voice_state_init(Voice_state* state, void (*init_state)(Voice_state*))
{
    assert(state != NULL);
    Voice_state_clear(state);
    state->active = true;
    state->note_on = true;
    if (init_state != NULL)
    {
        init_state(state);
    }
    return state;
}


Voice_state* Voice_state_clear(Voice_state* state)
{
    assert(state != NULL);
    state->active = false;
    state->freq = 0;
    state->ramp_attack = 0;
    state->ramp_release = 0;
    state->pos = 0;
    state->pos_rem = 0;
    state->rel_pos = 0;
    state->rel_pos_rem = 0;
    state->dir = 1;
    state->note_on = false;
    state->noff_pos = 0;
    state->noff_pos_rem = 0;
    state->pedal = false;
    state->on_ve_pos = 0;
    state->off_ve_pos = 0;
    return state;
}


