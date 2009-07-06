

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
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>

#include <status_line.h>
#include <peak_meter.h>


void get_minutes_seconds(uint64_t frames, uint32_t freq, int* minutes, double* seconds)
{
    assert(freq > 0);
    assert(minutes != NULL);
    assert(seconds != NULL);
    *minutes = (frames / freq / 60) % 60;
    *seconds = remainder((double)frames / freq, 60);
    if (*seconds < 0)
    {
        *seconds += 60;
    }
    return;
}


#define print_status(line, pos, max, ...)      \
    do                                         \
    {                                          \
        int printed = snprintf((line) + (pos), \
                               (max) - (pos),  \
                               __VA_ARGS__);   \
        if (printed >= 0)                      \
        {                                      \
            (pos) += printed;                  \
            if ((pos) >= (max))                \
            {                                  \
                (pos) = (max) - 1;             \
            }                                  \
        }                                      \
    } while (false)


int get_status_line(char* line,
                    int max_len,
                    kqt_Mix_state* mix_state,
                    int min_len,
                    uint64_t* clipped,
                    uint64_t frames_total,
                    uint16_t voices,
                    uint32_t freq)
{
    assert(line != NULL);
    assert(min_len >= 0);
    assert(max_len > 0);
    assert(min_len <= max_len);
    assert(mix_state != NULL);
    assert(clipped != NULL);
    assert(freq > 0);

    const int peak_meter_chars = 10;
    char peak_meter[10 * 6] = { '\0' };
    get_peak_meter(peak_meter, peak_meter_chars, mix_state, -40, -4, clipped);
    int excess_chars = strlen(peak_meter) - peak_meter_chars;

    uint32_t frames = mix_state->frames;
    int minutes = 0;
    double seconds = 0;
    get_minutes_seconds(frames, freq, &minutes, &seconds);

    uint64_t frames_left = frames_total - frames;
    int minutes_left = 0;
    double seconds_left = 0;
    get_minutes_seconds(frames_left, freq, &minutes_left, &seconds_left);

    int minutes_total = 0;
    double seconds_total = 0;
    get_minutes_seconds(frames_total, freq, &minutes_total, &seconds_total);

    double pos = kqt_Reltime_get_beats(&mix_state->pos) +
                 ((double)kqt_Reltime_get_rem(&mix_state->pos) / KQT_RELTIME_BEAT);

    int line_pos = 0;
    print_status(line, line_pos, max_len,
                 "%s"
                 "Subsong: %02" PRIu16
                 ", Time: %02d:%04.1f",
                 peak_meter,
                 mix_state->subsong,
                 minutes, seconds);
    if (frames_total >= mix_state->frames)
    {
        print_status(line, line_pos, max_len,
                     " [%02d:%04.1f]"
                     " of %02d:%04.1f",
                     minutes_left, seconds_left,
                     minutes_total, seconds_total);
    }
    print_status(line, line_pos, max_len,
                 ", Position: %02" PRIu16 "/%04.1f"
                 ", Voices: %" PRIu16 " (%" PRIu16 ")",
                 mix_state->order, pos,
                 mix_state->voices, voices);

    line_pos -= excess_chars;
    if (line_pos >= min_len)
    {
        min_len = line_pos;
    }
    else
    {
        for (int i = line_pos + excess_chars;
                i < min_len + excess_chars; ++i)
        {
            line[i] = ' ';
        }
    }
    line[min_len + excess_chars] = '\0';

    return min_len;
}


