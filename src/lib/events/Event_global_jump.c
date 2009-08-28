

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
#include <stdbool.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_global_jump.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc jump_desc[] =
{
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_global_jump_set(Event* event, int index, void* data);

static void* Event_global_jump_get(Event* event, int index);

static void Event_global_jump_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_jump,
                         EVENT_GLOBAL_JUMP,
                         jump_desc,
                         event->play_id = 0,
                         event->counter = 0,
                         event->subsong = -1,
                         event->section = -1,
                         Reltime_set(&event->position, 0, 0))


static void Event_global_jump_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_JUMP);
    assert(play != NULL);
    Event_global_jump* jump = (Event_global_jump*)event;
    if (jump->play_id != play->play_id)
    {
        if (play->jump_set_counter == 0)
        {
            return;
        }
        jump->play_id = play->play_id;
        jump->counter = play->jump_set_counter;
        jump->subsong = play->jump_set_subsong;
        jump->section = play->jump_set_section;
        Reltime_copy(&jump->position, &play->jump_set_position);
    }
    if (jump->counter > 0)
    {
        --jump->counter;
        play->jump = true;
        play->jump_subsong = jump->subsong;
        play->jump_section = jump->section;
        Reltime_copy(&play->jump_position, &jump->position);
    }
    return;
}


static bool Event_global_jump_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_JUMP);
    assert(data != NULL);
    (void)event;
    (void)index;
    (void)data;
    return false;
}


static void* Event_global_jump_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_JUMP);
    (void)event;
    (void)index;
    return NULL;
}


