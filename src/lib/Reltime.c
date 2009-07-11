

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

#include <kunquat/Reltime.h>


#ifndef NDEBUG
    #define kqt_Reltime_validate(r) assert((r) != NULL), assert((r)->rem >= 0), assert((r)->rem < KQT_RELTIME_BEAT)
#else
    #define kqt_Reltime_validate(r) ((void)0)
#endif


kqt_Reltime* kqt_Reltime_init(kqt_Reltime* r)
{
    assert(r != NULL);
    r->beats = 0;
    r->rem = 0;
    return r;
}


int kqt_Reltime_cmp(const kqt_Reltime* r1, const kqt_Reltime* r2)
{
    kqt_Reltime_validate(r1);
    kqt_Reltime_validate(r2);
    if (r1->beats < r2->beats)
        return -1;
    else if (r1->beats > r2->beats)
        return 1;
    else if (r1->rem < r2->rem)
        return -1;
    else if (r1->rem > r2->rem)
        return 1;
    return 0;
}


kqt_Reltime* kqt_Reltime_set(kqt_Reltime* r, long long beats, long rem)
{
    assert(r != NULL);
    assert(rem >= 0);
    assert(rem < KQT_RELTIME_BEAT);
    r->beats = beats;
    r->rem = rem;
    return r;
}


long long kqt_Reltime_get_beats(const kqt_Reltime* r)
{
    assert(r != NULL);
    return r->beats;
}


long kqt_Reltime_get_rem(const kqt_Reltime* r)
{
    assert(r != NULL);
    return r->rem;
}


kqt_Reltime* kqt_Reltime_add(kqt_Reltime* result, const kqt_Reltime* r1, const kqt_Reltime* r2)
{
    kqt_Reltime_validate(r1);
    kqt_Reltime_validate(r2);
    assert(result != NULL);
    result->beats = r1->beats + r2->beats;
    result->rem = r1->rem + r2->rem;
    if (result->rem >= KQT_RELTIME_BEAT)
    {
        ++result->beats;
        result->rem -= KQT_RELTIME_BEAT;
    }
    else if (result->rem < 0)
    {
        --result->beats;
        result->rem += KQT_RELTIME_BEAT;
    }
    assert(result->rem >= 0);
    assert(result->rem < KQT_RELTIME_BEAT);
    return result;
}


kqt_Reltime* kqt_Reltime_sub(kqt_Reltime* result, const kqt_Reltime* r1, const kqt_Reltime* r2)
{
    kqt_Reltime_validate(r1);
    kqt_Reltime_validate(r2);
    assert(result != NULL);
    result->beats = r1->beats - r2->beats;
    result->rem = r1->rem - r2->rem;
    if (result->rem < 0)
    {
        --result->beats;
        result->rem += KQT_RELTIME_BEAT;
    }
    else if (result->rem >= KQT_RELTIME_BEAT)
    {
        ++result->beats;
        result->rem -= KQT_RELTIME_BEAT;
    }
    assert(result->rem >= 0);
    assert(result->rem < KQT_RELTIME_BEAT);
    return result;
}


kqt_Reltime* kqt_Reltime_copy(kqt_Reltime* dest, const kqt_Reltime* src)
{
    assert(dest != NULL);
    kqt_Reltime_validate(src);
    dest->beats = src->beats;
    dest->rem = src->rem;
    return dest;
}


long kqt_Reltime_toframes(const kqt_Reltime* r,
                          double tempo,
                          long freq)
{
    kqt_Reltime_validate(r);
    assert(r->beats >= 0);
    assert(tempo > 0);
    assert(freq > 0);
    return (long)((r->beats
            + ((double)r->rem / KQT_RELTIME_BEAT)) * 60 * freq / tempo);
}


kqt_Reltime* kqt_Reltime_fromframes(kqt_Reltime* r,
                                    long frames,
                                    double tempo,
                                    long freq)
{
    assert(r != NULL);
    assert(tempo > 0);
    assert(freq > 0);
    double val = (double)frames * tempo / freq / 60;
    r->beats = (long long)val;
    r->rem = (long)((val - r->beats) * KQT_RELTIME_BEAT);
    return r;
}


