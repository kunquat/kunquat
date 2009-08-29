

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
#include <Event_global_set_jump_section.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc set_jump_section_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { -1, KQT_SECTIONS_MAX - 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_global_set_jump_section,
                                   EVENT_GLOBAL_SET_JUMP_SECTION,
                                   int64_t, section)


static void Event_global_set_jump_section_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_set_jump_section,
                         EVENT_GLOBAL_SET_JUMP_SECTION,
                         set_jump_section_desc,
                         event->section = -1)


static void Event_global_set_jump_section_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_JUMP_SECTION);
    assert(play != NULL);
    Event_global_set_jump_section* set_jump_section = (Event_global_set_jump_section*)event;
    play->jump_set_section = set_jump_section->section;
    return;
}


