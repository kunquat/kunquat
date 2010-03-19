

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include <String_buffer.h>
#include <Reltime.h>


#ifndef NDEBUG
    #define Reltime_validate(r) assert((r) != NULL), assert((r)->rem >= 0), assert((r)->rem < KQT_RELTIME_BEAT)
#else
    #define Reltime_validate(r) ((void)0)
#endif


Reltime* Reltime_init(Reltime* r)
{
    assert(r != NULL);
    r->beats = 0;
    r->rem = 0;
    return r;
}


bool Reltime_serialise(Reltime* r, String_buffer* sb)
{
    Reltime_validate(r);
    assert(sb != NULL);
    if (String_buffer_error(sb))
    {
        return false;
    }
    char r_buf[48] = { '\0' };
    snprintf(r_buf, 48, "[%" PRId64 ", %9" PRId32 "]", r->beats, r->rem);
    return String_buffer_append_string(sb, r_buf);
}


int Reltime_cmp(const Reltime* r1, const Reltime* r2)
{
    Reltime_validate(r1);
    Reltime_validate(r2);
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


Reltime* Reltime_set(Reltime* r, int64_t beats, int32_t rem)
{
    assert(r != NULL);
    assert(rem >= 0);
    assert(rem < KQT_RELTIME_BEAT);
    r->beats = beats;
    r->rem = rem;
    return r;
}


int64_t Reltime_get_beats(const Reltime* r)
{
    assert(r != NULL);
    return r->beats;
}


int32_t Reltime_get_rem(const Reltime* r)
{
    assert(r != NULL);
    return r->rem;
}


Reltime* Reltime_add(Reltime* result, const Reltime* r1, const Reltime* r2)
{
    Reltime_validate(r1);
    Reltime_validate(r2);
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


Reltime* Reltime_sub(Reltime* result, const Reltime* r1, const Reltime* r2)
{
    Reltime_validate(r1);
    Reltime_validate(r2);
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


Reltime* Reltime_copy(Reltime* dest, const Reltime* src)
{
    assert(dest != NULL);
    Reltime_validate(src);
    dest->beats = src->beats;
    dest->rem = src->rem;
    return dest;
}


uint32_t Reltime_toframes(const Reltime* r,
                          double tempo,
                          uint32_t freq)
{
    Reltime_validate(r);
    assert(r->beats >= 0);
    assert(tempo > 0);
    assert(freq > 0);
    double frame_count = ((r->beats
            + ((double)r->rem / KQT_RELTIME_BEAT)) * 60 * freq / tempo);
    if (frame_count > UINT32_MAX)
    {
        return UINT32_MAX;
    }
    return (uint32_t)frame_count;
}


Reltime* Reltime_fromframes(Reltime* r,
                            uint32_t frames,
                            double tempo,
                            uint32_t freq)
{
    assert(r != NULL);
    assert(tempo > 0);
    assert(freq > 0);
    double val = (double)frames * tempo / freq / 60;
    r->beats = (int64_t)val;
    r->rem = (int32_t)((val - r->beats) * KQT_RELTIME_BEAT);
    return r;
}


