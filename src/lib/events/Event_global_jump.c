

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
#include <limits.h>

#include <Event_common.h>
#include <Event_global_jump.h>
#include <File_base.h>
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


Event_create_constructor(Event_global_jump,
                         EVENT_GLOBAL_JUMP,
                         jump_desc,
                         event->play_id = 0,
                         event->counter = 0,
                         event->subsong = -1,
                         event->section = -1,
                         Reltime_set(&event->row, 0, 0));


bool Event_global_jump_process(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    (void)fields;
    global_state->jump = true;
    return true;
}


void Trigger_global_jump_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_JUMP);
    assert(play != NULL);
    Event_global_jump* jump = (Event_global_jump*)event;
    if (jump->play_id != play->play_id)
    {
        if (play->jump_set_counter == 0)
        {
            jump->play_id = 0;
            return;
        }
        jump->play_id = play->play_id;
        jump->counter = play->jump_set_counter;
        jump->subsong = play->jump_set_subsong;
        jump->section = play->jump_set_section;
        Reltime_copy(&jump->row, &play->jump_set_row);
    }
    if (jump->counter > 0)
    {
        --jump->counter;
        play->jump = true;
        play->jump_subsong = jump->subsong;
        play->jump_section = jump->section;
        Reltime_copy(&play->jump_row, &jump->row);
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


