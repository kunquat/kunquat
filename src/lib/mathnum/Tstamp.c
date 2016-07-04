

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <mathnum/Tstamp.h>

#include <debug/assert.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef NDEBUG
    #define Tstamp_validate(ts)               \
    if (true)                                 \
    {                                         \
        rassert((ts) != NULL);                \
        rassert((ts)->rem >= 0);              \
        rassert((ts)->rem < KQT_TSTAMP_BEAT); \
    } else (void)0
#else
    #define Tstamp_validate(ts) (ignore(ts))
#endif


Tstamp* Tstamp_init(Tstamp* ts)
{
    rassert(ts != NULL);

    ts->beats = 0;
    ts->rem = 0;

    return ts;
}


int Tstamp_cmp(const Tstamp* ts1, const Tstamp* ts2)
{
    Tstamp_validate(ts1);
    Tstamp_validate(ts2);

    if (ts1->beats < ts2->beats)
        return -1;
    else if (ts1->beats > ts2->beats)
        return 1;
    else if (ts1->rem < ts2->rem)
        return -1;
    else if (ts1->rem > ts2->rem)
        return 1;
    return 0;
}


Tstamp* Tstamp_set(Tstamp* ts, int64_t beats, int32_t rem)
{
    rassert(ts != NULL);
    rassert(rem >= 0);
    rassert(rem < KQT_TSTAMP_BEAT);

    ts->beats = beats;
    ts->rem = rem;

    return ts;
}


int64_t Tstamp_get_beats(const Tstamp* ts)
{
    rassert(ts != NULL);
    return ts->beats;
}


int32_t Tstamp_get_rem(const Tstamp* ts)
{
    rassert(ts != NULL);
    return ts->rem;
}


Tstamp* Tstamp_add(Tstamp* result, const Tstamp* ts1, const Tstamp* ts2)
{
    rassert(result != NULL);
    Tstamp_validate(ts1);
    Tstamp_validate(ts2);

    result->beats = ts1->beats + ts2->beats;
    result->rem = ts1->rem + ts2->rem;
    if (result->rem >= KQT_TSTAMP_BEAT)
    {
        ++result->beats;
        result->rem -= (int32_t)KQT_TSTAMP_BEAT;
    }
    else if (result->rem < 0)
    {
        --result->beats;
        result->rem += (int32_t)KQT_TSTAMP_BEAT;
    }
    rassert(result->rem >= 0);
    rassert(result->rem < KQT_TSTAMP_BEAT);

    return result;
}


Tstamp* Tstamp_sub(Tstamp* result, const Tstamp* ts1, const Tstamp* ts2)
{
    rassert(result != NULL);
    Tstamp_validate(ts1);
    Tstamp_validate(ts2);

    result->beats = ts1->beats - ts2->beats;
    result->rem = ts1->rem - ts2->rem;
    if (result->rem < 0)
    {
        --result->beats;
        result->rem += (int32_t)KQT_TSTAMP_BEAT;
    }
    else if (result->rem >= KQT_TSTAMP_BEAT)
    {
        ++result->beats;
        result->rem -= (int32_t)KQT_TSTAMP_BEAT;
    }
    rassert(result->rem >= 0);
    rassert(result->rem < KQT_TSTAMP_BEAT);

    return result;
}


Tstamp* Tstamp_copy(Tstamp* dest, const Tstamp* src)
{
    rassert(dest != NULL);
    Tstamp_validate(src);

    dest->beats = src->beats;
    dest->rem = src->rem;

    return dest;
}


Tstamp* Tstamp_min(Tstamp* result, const Tstamp* ts1, const Tstamp* ts2)
{
    rassert(result != NULL);
    Tstamp_validate(ts1);
    Tstamp_validate(ts2);

    if (Tstamp_cmp(ts1, ts2) < 0)
        Tstamp_copy(result, ts1);
    else
        Tstamp_copy(result, ts2);

    return result;
}


double Tstamp_toframes(const Tstamp* ts, double tempo, int32_t rate)
{
    Tstamp_validate(ts);
    rassert(ts->beats >= 0);
    rassert(tempo > 0);
    rassert(rate > 0);

    return ((double)ts->beats + ((double)ts->rem / KQT_TSTAMP_BEAT)) *
            60 * rate / tempo;
}


Tstamp* Tstamp_fromframes(Tstamp* ts, int32_t frames, double tempo, int32_t rate)
{
    rassert(ts != NULL);
    rassert(frames >= 0);
    rassert(tempo > 0);
    rassert(rate > 0);

    double val = (double)frames * tempo / rate / 60;
    ts->beats = (int64_t)val;
    ts->rem = (int32_t)((val - (double)ts->beats) * KQT_TSTAMP_BEAT);

    return ts;
}


