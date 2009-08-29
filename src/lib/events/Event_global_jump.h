

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


#ifndef K_EVENT_GLOBAL_JUMP_H
#define K_EVENT_GLOBAL_JUMP_H


#include <stdint.h>

#include <Event_global.h>
#include <Reltime.h>


typedef struct Event_global_jump
{
    Event_global parent;
    uint64_t play_id;
    int64_t counter;
    int16_t subsong;
    int16_t section;
    Reltime row;
} Event_global_jump;


Event* new_Event_global_jump(Reltime* pos);


#endif // K_EVENT_GLOBAL_JUMP_H


