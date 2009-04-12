

/*
 * Copyright 2008 Tomi Jylhä-Ollila
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


#ifndef K_SONG_LIMITS_H
#define K_SONG_LIMITS_H


#define SONG_NAME_MAX (128)

#define SUBSONGS_MAX (256)
#define ORDERS_MAX (256)

#define PATTERNS_MAX (1024)

#define COLUMNS_MAX (64)

#define INSTRUMENTS_MAX (255)
#define INS_NAME_MAX (64)
#define GENERATORS_MAX (8)

#define BUF_COUNT_MAX (2)

#define NOTE_TABLES_MAX (16)

#define NOTE_TABLE_NAME_MAX (256)
#define NOTE_TABLE_NOTE_NAME_MAX (16)
#define NOTE_TABLE_NOTE_MOD_NAME_MAX (8)
#define NOTE_TABLE_OCTAVES (16)
#define NOTE_TABLE_MIDDLE_OCTAVE_UNBIASED (8)
#define NOTE_TABLE_NOTE_MODS (16)
#define NOTE_TABLE_NOTES (128)
#define NOTE_TABLE_OCTAVE_BIAS (-3)
#define NOTE_TABLE_OCTAVE_FIRST (NOTE_TABLE_OCTAVE_BIAS)
#define NOTE_TABLE_MIDDLE_OCTAVE (NOTE_TABLE_MIDDLE_OCTAVE_UNBIASED + NOTE_TABLE_OCTAVE_BIAS)
#define NOTE_TABLE_OCTAVE_LAST (NOTE_TABLE_OCTAVES - 1 + NOTE_TABLE_OCTAVE_BIAS)

#define EVENT_FIELDS (4)


#endif // K_SONG_LIMITS_H


