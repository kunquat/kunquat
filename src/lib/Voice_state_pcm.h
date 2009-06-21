

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


#ifndef K_VOICE_STATE_PCM_H
#define K_VOICE_STATE_PCM_H


#include <Voice_state.h>


typedef struct Voice_state_pcm
{
    Voice_state parent;
    int sample;
    double freq;
    double volume;
    uint8_t source;
    uint8_t style;
    double middle_tone;
} Voice_state_pcm;


#endif // K_VOICE_STATE_PCM_H


