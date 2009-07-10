

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
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <kunquat/Mix_state.h>
#include <peak_meter.h>


char* get_peak_meter(char* str,
                     int len,
                     kqt_Mix_state* mix_state,
                     double lower,
                     double upper,
                     uint64_t* clipped,
                     bool unicode)
{
    assert(str != NULL);
    assert(len > 0);
    assert(mix_state != NULL);
    assert(isfinite(lower));
    assert(isfinite(upper));
    assert(lower < upper);
    assert(clipped != NULL);
    double vols[2] = { -INFINITY, -INFINITY };
    for (int i = 0; i < 2; ++i)
    {
        if (mix_state->min_amps[i] < mix_state->max_amps[i])
        {
            vols[i] = mix_state->max_amps[i] - mix_state->min_amps[i];
            vols[i] = log2(vols[i] / 2) * 6;
        }
    }
    len -= 3;
    int sizes[2] = { 0 };
    int scale = len * 2;
    double scale_dB = upper - lower;
    for (int i = 0; i < 2; ++i)
    {
        sizes[i] = (vols[i] - lower) * scale / scale_dB;
    }
    char* pos = str;
    strcpy(pos, "[");
    pos += strlen(pos);
    if (unicode)
    {
        for (int i = 0; i < len; ++i)
        {
            char block[7] = { '\0' };
            if (i * 2 + 2 <= sizes[0])
            {
                // ▀
                if (i * 2 + 2 <= sizes[1])
                {
                    strcpy(block, "█");
                }
                else if (i * 2 + 1 <= sizes[1])
                {
                    strcpy(block, "▛");
                }
                else
                {
                    strcpy(block, "▀");
                }
            }
            else if (i * 2 + 1 <= sizes[0])
            {
                // ▘
                if (i * 2 + 2 <= sizes[1])
                {
                    strcpy(block, "▙");
                }
                else if (i * 2 + 1 <= sizes[1])
                {
                    strcpy(block, "▌");
                }
                else
                {
                    strcpy(block, "▘");
                }
            }
            else
            {
                // 
                if (i * 2 + 2 <= sizes[1])
                {
                    strcpy(block, "▄");
                }
                else if (i * 2 + 1 <= sizes[1])
                {
                    strcpy(block, "▖");
                }
                else
                {
                    strcpy(block, " ");
                }
            }
            strcpy(pos, block);
            pos += strlen(block);
            assert((pos - str) < (len * 6 + 6));
        }
    }
    else
    {
        for (int i = 0; i < len; ++i)
        {
            char block = ' ';
            if (i * 2 + 2 <= sizes[0])
            {
                block = '#';
            }
            else if (i * 2 + 1 <= sizes[0])
            {
                if (i * 2 + 1 <= sizes[1])
                {
                    block = '#';
                }
            }
            else
            {
                if (i * 2 + 2 <= sizes[1])
                {
                    block = '#';
                }
            }
            *pos = block;
            ++pos;
            *pos = '\0';
            assert((pos - str) < (len * 6 + 6));
        }
    }

    strcpy(pos, "]");
    pos += strlen(pos);

    if (unicode)
    {
        char clip[7] = { '\0' };
        if (clipped[0] > 0 && clipped[1] > 0)
        {
            strcpy(clip, "▌");
        }
        else if (clipped[0] > 0)
        {
            strcpy(clip, "▘");
        }
        else if (clipped[1] > 0)
        {
            strcpy(clip, "▖");
        }
        else
        {
            strcpy(clip, " ");
        }
        strcpy(pos, clip);
        pos += strlen(clip);
    }
    else
    {
        *pos = ' ';
        if (clipped[0] > 0 || clipped[1] > 0)
        {
            *pos = '!';
        }
        ++pos;
    }
    *pos = '\0';
    return str;
}


