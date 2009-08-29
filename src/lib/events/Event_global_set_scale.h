

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


#ifndef K_EVENT_GLOBAL_SET_SCALE_H
#define K_EVENT_GLOBAL_SET_SCALE_H


#include <Event_global.h>
#include <Reltime.h>


typedef struct Event_global_set_scale
{
    Event_global parent;
    int64_t scale_index;
} Event_global_set_scale;


Event* new_Event_global_set_scale(Reltime* pos);


#endif // K_EVENT_GLOBAL_SET_SCALE_H


