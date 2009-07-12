

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


#ifndef KQT_LIMITS_H
#define KQT_LIMITS_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Maximum number of Kunquat Handles that can be open simultaneously.
 */
#define KQT_HANDLES_MAX (256)


/**
 * Maximum number of output buffers a Kunquat Handle can contain.
 */
#define KQT_BUFFERS_MAX (2)


/**
 * Maximum number of Voices used for mixing.
 */
#define KQT_VOICES_MAX (1024)


/**
 * Maximum number of Subsongs in a Kunquat Handle.
 */
#define KQT_SUBSONGS_MAX (256)


/**
 * Maximum number of sections in a Subsong.
 */
#define KQT_SECTIONS_MAX (256)


/**
 * Maximum number of Patterns in a Kunquat Handle.
 */
#define KQT_PATTERNS_MAX (1024)


/**
 * Maximum number of Voice Columns in a Kunquat Handle (Global Column not
 * included).
 */
#define KQT_COLUMNS_MAX (64)


/**
 * Maximum number of Instruments in a Kunquat Handle (Note: Instrument
 * indices start at 1).
 */
#define KQT_INSTRUMENTS_MAX (255)


/**
 * Maximum number of Generators in a Kunquat Instruemnt.
 */
#define KQT_GENERATORS_MAX (8)


/**
 * Maximum number of Scales in a Kunquat Handle.
 */
#define KQT_SCALES_MAX (16)

#define KQT_SCALE_OCTAVES (16)
#define KQT_SCALE_MIDDLE_OCTAVE_UNBIASED (8)
#define KQT_SCALE_NOTE_MODS (16)
#define KQT_SCALE_NOTES (128)
#define KQT_SCALE_OCTAVE_BIAS (-3)
#define KQT_SCALE_OCTAVE_FIRST (KQT_SCALE_OCTAVE_BIAS)
#define KQT_SCALE_MIDDLE_OCTAVE (KQT_SCALE_MIDDLE_OCTAVE_UNBIASED + KQT_SCALE_OCTAVE_BIAS)
#define KQT_SCALE_OCTAVE_LAST (KQT_SCALE_OCTAVES - 1 + KQT_SCALE_OCTAVE_BIAS)


#ifdef __cplusplus
}
#endif


#endif // KQT_LIMITS_H


