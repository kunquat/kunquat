

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
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <float.h>

#include <check.h>

#include <Reltime.h>


Suite* Reltime_suite(void);


START_TEST (init)
{
    Reltime rel;
    Reltime* rp = Reltime_init(&rel);
    fail_unless(rp == &rel,
            "Reltime_init() returned %p instead of %p.", rp, &rel);
    fail_unless(rp->beats == 0,
            "Reltime_init() set beats to %lld instead of 0.", (long long)rp->beats);
    fail_unless(rp->rem == 0,
            "Reltime_init() set remainder to %ld instead of 0.", (long)rp->rem);
}
END_TEST

#ifndef NDEBUG
START_TEST (init_break)
{
    Reltime_init(NULL);
}
END_TEST
#endif

START_TEST (set)
{
    int64_t beat_values[] = { INT64_MIN, INT64_MIN + 1, -1, 0, 1, INT64_MAX - 1, INT64_MAX };
    int32_t part_values[] = { 0, 1, RELTIME_BEAT - 1 };
    int32_t all_parts[] = { INT32_MIN, INT32_MIN + 1, -1, 0, 1,
            RELTIME_BEAT - 1, RELTIME_BEAT, INT32_MAX - 1, INT32_MAX };
    for (size_t i = 0; i < sizeof(beat_values) / sizeof(int64_t); ++i)
    {
        for (size_t k = 0; k < sizeof(part_values) / sizeof(int32_t); ++k)
        {
            for (size_t l = 0; l < sizeof(all_parts) / sizeof(int32_t); ++l)
            {
                Reltime* r = Reltime_init(&(Reltime){ .beats = 0 });
                r->rem = all_parts[l];
                Reltime* s = Reltime_set(r, beat_values[i], part_values[k]);
                fail_unless(s == r,
                        "Reltime_set() returned %p instead of %p.", s, r);
                fail_unless(s->beats == beat_values[i],
                        "Reltime_set() set beats to %lld instead of %lld.",
                        (long long)s->beats, (long long)beat_values[i]);
                fail_unless(s->rem == part_values[k],
                        "Reltime_set() set part to %lld instead of %lld.",
                        (long long)s->rem, (long long)part_values[k]);
            }
        }
    }
}
END_TEST

#ifndef NDEBUG
START_TEST (set_break_reltime)
{
    Reltime_set(NULL, 0, 0);
}
END_TEST

START_TEST (set_break_part1)
{
    Reltime_set(&(Reltime){ .beats = 0 }, 0, INT32_MIN);
}
END_TEST

START_TEST (set_break_part2)
{
    Reltime_set(&(Reltime){ .beats = 0 }, 0, -1);
}
END_TEST

START_TEST (set_break_part3)
{
    Reltime_set(&(Reltime){ .beats = 0 }, 0, RELTIME_BEAT);
}
END_TEST

START_TEST (set_break_part4)
{
    Reltime_set(&(Reltime){ .beats = 0 }, 0, INT32_MAX);
}
END_TEST
#endif

START_TEST (cmp)
{
    Reltime* r1 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime* r2 = Reltime_init(&(Reltime){ .beats = 0 });
    int res = 0;
#define CMPTEXT(c) ((c) < 0 ? "smaller" : ((c) > 0 ? "greater" : "equal"))

    // beats and parts equal
    Reltime_set(r1, INT64_MIN, 0);
    Reltime_set(r2, INT64_MIN, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));
    Reltime_set(r1, INT64_MIN, RELTIME_BEAT - 1);
    Reltime_set(r2, INT64_MIN, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));
    
    Reltime_set(r1, INT64_MIN + 1, 0);
    Reltime_set(r2, INT64_MIN + 1, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));
    Reltime_set(r1, INT64_MIN + 1, RELTIME_BEAT - 1);
    Reltime_set(r2, INT64_MIN + 1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));
    
    Reltime_set(r1, -1, 0);
    Reltime_set(r2, -1, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));
    Reltime_set(r1, -1, RELTIME_BEAT - 1);
    Reltime_set(r2, -1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));
    
    Reltime_set(r1, 0, 0);
    Reltime_set(r2, 0, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));
    Reltime_set(r1, 0, RELTIME_BEAT - 1);
    Reltime_set(r2, 0, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));

    Reltime_set(r1, 1, 0);
    Reltime_set(r2, 1, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));
    Reltime_set(r1, 1, RELTIME_BEAT - 1);
    Reltime_set(r2, 1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));

    Reltime_set(r1, INT64_MAX - 1, 0);
    Reltime_set(r2, INT64_MAX - 1, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));
    Reltime_set(r1, INT64_MAX - 1, RELTIME_BEAT - 1);
    Reltime_set(r2, INT64_MAX - 1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));

    Reltime_set(r1, INT64_MAX, 0);
    Reltime_set(r2, INT64_MAX, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));
    Reltime_set(r1, INT64_MAX, RELTIME_BEAT - 1);
    Reltime_set(r2, INT64_MAX, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res == 0,
            "Reltime_cmp() returned %s instead of equal.", CMPTEXT(res));

    // beats equal and parts unequal
    Reltime_set(r1, INT64_MIN, 0);
    Reltime_set(r2, INT64_MIN, 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, INT64_MIN, 0);
    Reltime_set(r2, INT64_MIN, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, INT64_MIN, RELTIME_BEAT - 2);
    Reltime_set(r2, INT64_MIN, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));

    Reltime_set(r1, -1, 0);
    Reltime_set(r2, -1, 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, -1, 0);
    Reltime_set(r2, -1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, -1, RELTIME_BEAT - 2);
    Reltime_set(r2, -1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));

    Reltime_set(r1, 0, 0);
    Reltime_set(r2, 0, 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, 0, 0);
    Reltime_set(r2, 0, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, 0, RELTIME_BEAT - 2);
    Reltime_set(r2, 0, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));

    Reltime_set(r1, 1, 0);
    Reltime_set(r2, 1, 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, 1, 0);
    Reltime_set(r2, 1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, 1, RELTIME_BEAT - 2);
    Reltime_set(r2, 1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));

    Reltime_set(r1, INT64_MAX, 0);
    Reltime_set(r2, INT64_MAX, 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, INT64_MAX, 0);
    Reltime_set(r2, INT64_MAX, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, INT64_MAX, RELTIME_BEAT - 2);
    Reltime_set(r2, INT64_MAX, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));

    // beats unequal and parts equal
    Reltime_set(r1, INT64_MIN, 0);
    Reltime_set(r2, INT64_MIN + 1, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, INT64_MIN, RELTIME_BEAT - 1);
    Reltime_set(r2, INT64_MIN + 1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));

    Reltime_set(r1, -1, 0);
    Reltime_set(r2, 0, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, -1, RELTIME_BEAT - 1);
    Reltime_set(r2, 0, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));

    Reltime_set(r1, 0, 0);
    Reltime_set(r2, 1, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, 0, RELTIME_BEAT - 1);
    Reltime_set(r2, 1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));

    Reltime_set(r1, INT64_MAX - 1, 0);
    Reltime_set(r2, INT64_MAX, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, INT64_MAX - 1, RELTIME_BEAT - 1);
    Reltime_set(r2, INT64_MAX, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));

    Reltime_set(r1, INT64_MIN, 0);
    Reltime_set(r2, INT64_MAX, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, INT64_MIN, RELTIME_BEAT - 1);
    Reltime_set(r2, INT64_MAX, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));

    // beats and parts unequal
    Reltime_set(r1, INT64_MIN, 0);
    Reltime_set(r2, INT64_MIN + 1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, INT64_MIN, RELTIME_BEAT - 1);
    Reltime_set(r2, INT64_MIN + 1, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    
    Reltime_set(r1, -1, 0);
    Reltime_set(r2, 0, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, -1, RELTIME_BEAT - 1);
    Reltime_set(r2, 0, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    
    Reltime_set(r1, 0, 0);
    Reltime_set(r2, 1, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, 0, RELTIME_BEAT - 1);
    Reltime_set(r2, 1, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    
    Reltime_set(r1, INT64_MAX - 1, 0);
    Reltime_set(r2, INT64_MAX, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, INT64_MAX - 1, RELTIME_BEAT - 1);
    Reltime_set(r2, INT64_MAX, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    
    Reltime_set(r1, INT64_MIN, 0);
    Reltime_set(r2, INT64_MAX, RELTIME_BEAT - 1);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
    Reltime_set(r1, INT64_MIN, RELTIME_BEAT - 1);
    Reltime_set(r2, INT64_MAX, 0);
    res = Reltime_cmp(r1, r2);
    fail_unless(res < 0,
            "Reltime_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Reltime_cmp(r2, r1);
    fail_unless(res > 0,
            "Reltime_cmp() returned %s instead of greater.", CMPTEXT(res));
}
END_TEST

#ifndef NDEBUG
START_TEST (cmp_break_null1)
{
    Reltime_cmp(NULL, Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (cmp_break_null2)
{
    Reltime_cmp(Reltime_init(&(Reltime){ .beats = 0 }), NULL);
}
END_TEST

START_TEST (cmp_break_inv11)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MIN };
    Reltime_cmp(br, Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (cmp_break_inv12)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = -1 };
    Reltime_cmp(br, Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (cmp_break_inv13)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = RELTIME_BEAT };
    Reltime_cmp(br, Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (cmp_break_inv14)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MAX };
    Reltime_cmp(br, Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (cmp_break_inv21)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MIN };
    Reltime_cmp(Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST

START_TEST (cmp_break_inv22)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = -1 };
    Reltime_cmp(Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST

START_TEST (cmp_break_inv23)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = RELTIME_BEAT };
    Reltime_cmp(Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST

START_TEST (cmp_break_inv24)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MAX };
    Reltime_cmp(Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST
#endif


START_TEST (add)
{
    Reltime* ret = NULL;
    Reltime* res = Reltime_init(RELTIME_AUTO);
    res->rem = -1;
    Reltime* r1 = Reltime_init(RELTIME_AUTO);
    Reltime* r2 = Reltime_init(RELTIME_AUTO);
    Reltime* exp = Reltime_init(RELTIME_AUTO);

    Reltime_set(r1, -1, 0);
    Reltime_set(r2, -1, 1);
    Reltime_set(exp, -2, 1);
    ret = Reltime_add(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_add(res, r2, r1);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, -1, 0);
    Reltime_set(r2, 0, 1);
    Reltime_set(exp, -1, 1);
    ret = Reltime_add(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_add(res, r2, r1);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, -1, RELTIME_BEAT - 1);
    Reltime_set(r2, 0, 1);
    Reltime_set(exp, 0, 0);
    ret = Reltime_add(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_add(res, r2, r1);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, 0, RELTIME_BEAT - 1);
    Reltime_set(r2, 0, RELTIME_BEAT - 1);
    Reltime_set(exp, 1, RELTIME_BEAT - 2);
    ret = Reltime_add(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_add(res, r2, r1);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, -1, 0);
    Reltime_set(r2, 0, 0);
    Reltime_set(exp, -1, 0);
    ret = Reltime_add(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_add(res, r2, r1);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, 0, 0);
    Reltime_set(r2, 0, 0);
    Reltime_set(exp, 0, 0);
    ret = Reltime_add(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_add(res, r2, r1);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, 1, 0);
    Reltime_set(r2, 0, 0);
    Reltime_set(exp, 1, 0);
    ret = Reltime_add(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_add(res, r2, r1);
    fail_unless(ret == res,
            "Reltime_add() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
}
END_TEST

#ifndef NDEBUG
START_TEST (add_break_null1)
{
    Reltime* r1 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime* r2 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime_add(NULL, r1, r2);
}
END_TEST

START_TEST (add_break_null2)
{
    Reltime* r1 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime* r2 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime_add(r1, NULL, r2);
}
END_TEST

START_TEST (add_break_null3)
{
    Reltime* r1 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime* r2 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime_add(r1, r2, NULL);
}
END_TEST

START_TEST (add_break_inv21)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MIN };
    Reltime_add(Reltime_init(&(Reltime){ .beats = 0 }), br,
            Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (add_break_inv22)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = -1 };
    Reltime_add(Reltime_init(&(Reltime){ .beats = 0 }), br,
            Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (add_break_inv23)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = RELTIME_BEAT };
    Reltime_add(Reltime_init(&(Reltime){ .beats = 0 }), br,
            Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (add_break_inv24)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MAX };
    Reltime_add(Reltime_init(&(Reltime){ .beats = 0 }), br,
            Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (add_break_inv31)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MIN };
    Reltime_add(Reltime_init(&(Reltime){ .beats = 0 }),
            Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST

START_TEST (add_break_inv32)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = -1 };
    Reltime_add(Reltime_init(&(Reltime){ .beats = 0 }),
            Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST

START_TEST (add_break_inv33)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = RELTIME_BEAT };
    Reltime_add(Reltime_init(&(Reltime){ .beats = 0 }),
            Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST

START_TEST (add_break_inv34)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MAX };
    Reltime_add(Reltime_init(&(Reltime){ .beats = 0 }),
            Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST
#endif


START_TEST (sub)
{
    Reltime* ret = NULL;
    Reltime* res = Reltime_init(RELTIME_AUTO);
    res->rem = -1;
    Reltime* r1 = Reltime_init(RELTIME_AUTO);
    Reltime* r2 = Reltime_init(RELTIME_AUTO);
    Reltime* exp = Reltime_init(RELTIME_AUTO);

    Reltime_set(r1, -1, 0);
    Reltime_set(r2, -1, 1);
    Reltime_set(exp, -1, RELTIME_BEAT - 1);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, -1, 0);
    Reltime_set(r2, 0, 1);
    Reltime_set(exp, -2, RELTIME_BEAT - 1);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, -1, RELTIME_BEAT - 1);
    Reltime_set(r2, 0, 1);
    Reltime_set(exp, -1, RELTIME_BEAT - 2);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, 0, RELTIME_BEAT - 1);
    Reltime_set(r2, 0, RELTIME_BEAT - 1);
    Reltime_set(exp, 0, 0);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, -1, 0);
    Reltime_set(r2, 0, 0);
    Reltime_set(exp, -1, 0);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, 0, 0);
    Reltime_set(r2, 0, 0);
    Reltime_set(exp, 0, 0);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, 1, 0);
    Reltime_set(r2, 0, 0);
    Reltime_set(exp, 1, 0);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, 0, 0);
    Reltime_set(r2, -1, 0);
    Reltime_set(exp, 1, 0);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, 0, 0);
    Reltime_set(r2, 1, 0);
    Reltime_set(exp, -1, 0);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, 0, 0);
    Reltime_set(r2, 0, 1);
    Reltime_set(exp, -1, RELTIME_BEAT - 1);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(r1, 0, 0);
    Reltime_set(r2, -1, RELTIME_BEAT - 1);
    Reltime_set(exp, 0, 1);
    ret = Reltime_sub(res, r1, r2);
    fail_unless(ret == res,
            "Reltime_sub() returned %p instead of %p.", ret, res);
    fail_unless(Reltime_cmp(res, exp) == 0,
            "Reltime_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
}
END_TEST

#ifndef NDEBUG
START_TEST (sub_break_null1)
{
    Reltime* r1 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime* r2 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime_sub(NULL, r1, r2);
}
END_TEST

START_TEST (sub_break_null2)
{
    Reltime* r1 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime* r2 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime_sub(r1, NULL, r2);
}
END_TEST

START_TEST (sub_break_null3)
{
    Reltime* r1 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime* r2 = Reltime_init(&(Reltime){ .beats = 0 });
    Reltime_sub(r1, r2, NULL);
}
END_TEST

START_TEST (sub_break_inv21)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MIN };
    Reltime_sub(Reltime_init(&(Reltime){ .beats = 0 }), br,
            Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (sub_break_inv22)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = -1 };
    Reltime_sub(Reltime_init(&(Reltime){ .beats = 0 }), br,
            Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (sub_break_inv23)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = RELTIME_BEAT };
    Reltime_sub(Reltime_init(&(Reltime){ .beats = 0 }), br,
            Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (sub_break_inv24)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MAX };
    Reltime_sub(Reltime_init(&(Reltime){ .beats = 0 }), br,
            Reltime_init(&(Reltime){ .beats = 0 }));
}
END_TEST

START_TEST (sub_break_inv31)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MIN };
    Reltime_sub(Reltime_init(&(Reltime){ .beats = 0 }),
            Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST

START_TEST (sub_break_inv32)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = -1 };
    Reltime_sub(Reltime_init(&(Reltime){ .beats = 0 }),
            Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST

START_TEST (sub_break_inv33)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = RELTIME_BEAT };
    Reltime_sub(Reltime_init(&(Reltime){ .beats = 0 }),
            Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST

START_TEST (sub_break_inv34)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MAX };
    Reltime_sub(Reltime_init(&(Reltime){ .beats = 0 }),
            Reltime_init(&(Reltime){ .beats = 0 }), br);
}
END_TEST
#endif


START_TEST (copy)
{
    Reltime* ret = NULL;
    Reltime* src = Reltime_init(RELTIME_AUTO);
    Reltime* dest = Reltime_init(RELTIME_AUTO);

    Reltime_set(src, INT64_MAX, RELTIME_BEAT - 1);
    ret = Reltime_copy(dest, src);
    fail_unless(ret == dest,
            "Reltime_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Reltime_cmp(dest, src) == 0,
            "Reltime_copy() didn't produce a copy equal to the original.");

    Reltime_set(src, INT64_MAX, 0);
    ret = Reltime_copy(dest, src);
    fail_unless(ret == dest,
            "Reltime_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Reltime_cmp(dest, src) == 0,
            "Reltime_copy() didn't produce a copy equal to the original.");

    Reltime_set(src, 1, 0);
    ret = Reltime_copy(dest, src);
    fail_unless(ret == dest,
            "Reltime_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Reltime_cmp(dest, src) == 0,
            "Reltime_copy() didn't produce a copy equal to the original.");

    Reltime_set(src, 0, RELTIME_BEAT - 1);
    ret = Reltime_copy(dest, src);
    fail_unless(ret == dest,
            "Reltime_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Reltime_cmp(dest, src) == 0,
            "Reltime_copy() didn't produce a copy equal to the original.");

    Reltime_set(src, 0, 0);
    ret = Reltime_copy(dest, src);
    fail_unless(ret == dest,
            "Reltime_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Reltime_cmp(dest, src) == 0,
            "Reltime_copy() didn't produce a copy equal to the original.");

    Reltime_set(src, -1, RELTIME_BEAT - 1);
    ret = Reltime_copy(dest, src);
    fail_unless(ret == dest,
            "Reltime_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Reltime_cmp(dest, src) == 0,
            "Reltime_copy() didn't produce a copy equal to the original.");

    Reltime_set(src, -1, 1);
    ret = Reltime_copy(dest, src);
    fail_unless(ret == dest,
            "Reltime_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Reltime_cmp(dest, src) == 0,
            "Reltime_copy() didn't produce a copy equal to the original.");

    Reltime_set(src, INT64_MIN, RELTIME_BEAT - 1);
    ret = Reltime_copy(dest, src);
    fail_unless(ret == dest,
            "Reltime_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Reltime_cmp(dest, src) == 0,
            "Reltime_copy() didn't produce a copy equal to the original.");

    Reltime_set(src, INT64_MIN, 0);
    ret = Reltime_copy(dest, src);
    fail_unless(ret == dest,
            "Reltime_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Reltime_cmp(dest, src) == 0,
            "Reltime_copy() didn't produce a copy equal to the original.");
}
END_TEST

#ifndef NDEBUG
START_TEST (copy_break_null1)
{
    Reltime_copy(NULL, Reltime_init(RELTIME_AUTO));
}
END_TEST

START_TEST (copy_break_null2)
{
    Reltime_copy(Reltime_init(RELTIME_AUTO), NULL);
}
END_TEST

START_TEST (copy_break_inv21)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MIN };
    Reltime_copy(Reltime_init(RELTIME_AUTO), br);
}
END_TEST

START_TEST (copy_break_inv22)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = -1 };
    Reltime_copy(Reltime_init(RELTIME_AUTO), br);
}
END_TEST

START_TEST (copy_break_inv23)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = RELTIME_BEAT };
    Reltime_copy(Reltime_init(RELTIME_AUTO), br);
}
END_TEST

START_TEST (copy_break_inv24)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MAX };
    Reltime_copy(Reltime_init(RELTIME_AUTO), br);
}
END_TEST
#endif


START_TEST (toframes)
{
    Reltime* r = Reltime_init(RELTIME_AUTO);
    uint32_t res = 0;
    res = Reltime_toframes(r, DBL_MIN, 1);
    fail_unless(res == 0,
            "Reltime_toframes() returned %ld instead of 0.", (long)res);
    res = Reltime_toframes(r, DBL_MIN, UINT32_MAX);
    fail_unless(res == 0,
            "Reltime_toframes() returned %ld instead of 0.", (long)res);
    res = Reltime_toframes(r, DBL_MAX, 1);
    fail_unless(res == 0,
            "Reltime_toframes() returned %ld instead of 0.", (long)res);
    res = Reltime_toframes(r, DBL_MAX, UINT32_MAX);
    fail_unless(res == 0,
            "Reltime_toframes() returned %ld instead of 0.", (long)res);

    Reltime_set(r, 1, 0);
    res = Reltime_toframes(r, 60, 44100);
    fail_unless(res == 44100,
            "Reltime_toframes() returned %ld instead of 44100.", (long)res);
    res = Reltime_toframes(r, 120, 44100);
    fail_unless(res == 22050,
            "Reltime_toframes() returned %ld instead of 22050.", (long)res);
    res = Reltime_toframes(r, 60, 96000);
    fail_unless(res == 96000,
            "Reltime_toframes() returned %ld instead of 96000.", (long)res);

    Reltime_set(r, 0, RELTIME_BEAT / 2);
    res = Reltime_toframes(r, 60, 44100);
    fail_unless(res == 22050,
            "Reltime_toframes() returned %ld instead of 22050.", (long)res);
    res = Reltime_toframes(r, 120, 44100);
    fail_unless(res == 11025,
            "Reltime_toframes() returned %ld instead of 11025.", (long)res);
    res = Reltime_toframes(r, 60, 96000);
    fail_unless(res == 48000,
            "Reltime_toframes() returned %ld instead of 48000.", (long)res);

    Reltime_set(r, 1, RELTIME_BEAT / 2);
    res = Reltime_toframes(r, 60, 44100);
    fail_unless(res == 66150,
            "Reltime_toframes() returned %ld instead of 66150.", (long)res);
    res = Reltime_toframes(r, 120, 44100);
    fail_unless(res == 33075,
            "Reltime_toframes() returned %ld instead of 33075.", (long)res);
    res = Reltime_toframes(r, 60, 96000);
    fail_unless(res == 144000,
            "Reltime_toframes() returned %ld instead of 144000.", (long)res);
}
END_TEST

#ifndef NDEBUG
START_TEST (toframes_break_null1)
{
    Reltime_toframes(NULL, 1, 1);
}
END_TEST

START_TEST (toframes_break_inv11)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MIN };
    Reltime_toframes(br, 1, 1);
}
END_TEST

START_TEST (toframes_break_inv12)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = -1 };
    Reltime_toframes(br, 1, 1);
}
END_TEST

START_TEST (toframes_break_inv13)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = RELTIME_BEAT };
    Reltime_toframes(br, 1, 1);
}
END_TEST

START_TEST (toframes_break_inv14)
{
    Reltime* br = &(Reltime){ .beats = 0, .rem = INT32_MAX };
    Reltime_toframes(br, 1, 1);
}
END_TEST

START_TEST (toframes_break_inv15)
{
    Reltime_toframes(Reltime_set(RELTIME_AUTO, INT64_MIN, 0), 1, 1);
}
END_TEST

START_TEST (toframes_break_inv16)
{
    Reltime_toframes(Reltime_set(RELTIME_AUTO, -1, RELTIME_BEAT - 1), 1, 1);
}
END_TEST

START_TEST (toframes_break_inv21)
{
    Reltime_toframes(Reltime_init(RELTIME_AUTO), -DBL_MAX, 1);
}
END_TEST

START_TEST (toframes_break_inv22)
{
    Reltime_toframes(Reltime_init(RELTIME_AUTO), 0, 1);
}
END_TEST

START_TEST (toframes_break_inv31)
{
    Reltime_toframes(Reltime_init(RELTIME_AUTO), 1, 0);
}
END_TEST
#endif


START_TEST (fromframes)
{
    Reltime* r = &(Reltime){ .beats = 0, .rem = -1 };
    Reltime* ret = NULL;
    Reltime* exp = Reltime_init(RELTIME_AUTO);

    ret = Reltime_fromframes(r, 0, DBL_MIN, 1);
    fail_unless(ret == r,
            "Reltime_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Reltime_cmp(r, exp) == 0,
            "Reltime_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_fromframes(r, 0, DBL_MIN, UINT32_MAX);
    fail_unless(ret == r,
            "Reltime_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Reltime_cmp(r, exp) == 0,
            "Reltime_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_fromframes(r, 0, DBL_MAX, UINT32_MAX);
    fail_unless(ret == r,
            "Reltime_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Reltime_cmp(r, exp) == 0,
            "Reltime_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_fromframes(r, 0, DBL_MAX, UINT32_MAX);
    fail_unless(ret == r,
            "Reltime_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Reltime_cmp(r, exp) == 0,
            "Reltime_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(exp, 1, 0);
    ret = Reltime_fromframes(r, 44100, 60, 44100);
    fail_unless(ret == r,
            "Reltime_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Reltime_cmp(r, exp) == 0,
            "Reltime_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_fromframes(r, 48000, 120, 96000);
    fail_unless(ret == r,
            "Reltime_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Reltime_cmp(r, exp) == 0,
            "Reltime_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(exp, 0, RELTIME_BEAT / 2);
    ret = Reltime_fromframes(r, 22050, 60, 44100);
    fail_unless(ret == r,
            "Reltime_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Reltime_cmp(r, exp) == 0,
            "Reltime_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_fromframes(r, 24000, 120, 96000);
    fail_unless(ret == r,
            "Reltime_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Reltime_cmp(r, exp) == 0,
            "Reltime_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);

    Reltime_set(exp, 1, RELTIME_BEAT / 2);
    ret = Reltime_fromframes(r, 66150, 60, 44100);
    fail_unless(ret == r,
            "Reltime_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Reltime_cmp(r, exp) == 0,
            "Reltime_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Reltime_fromframes(r, 72000, 120, 96000);
    fail_unless(ret == r,
            "Reltime_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Reltime_cmp(r, exp) == 0,
            "Reltime_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
}
END_TEST

#ifndef NDEBUG
START_TEST (fromframes_break_null1)
{
    Reltime_fromframes(NULL, 0, 1, 1);
}
END_TEST

START_TEST (fromframes_break_inv_tempo1)
{
    Reltime_fromframes(Reltime_init(RELTIME_AUTO), 0, -DBL_MAX, 1);
}
END_TEST

START_TEST (fromframes_break_inv_tempo2)
{
    Reltime_fromframes(Reltime_init(RELTIME_AUTO), 0, 0, 1);
}
END_TEST

START_TEST (fromframes_break_inv_freq)
{
    Reltime_fromframes(Reltime_init(RELTIME_AUTO), 0, 1, 0);
}
END_TEST
#endif


Suite* Reltime_suite(void)
{
    Suite* s = suite_create("Reltime");
    TCase* tc_init = tcase_create("init");
    TCase* tc_set = tcase_create("set");
    TCase* tc_cmp = tcase_create("cmp");
    TCase* tc_add = tcase_create("add");
    TCase* tc_sub = tcase_create("sub");
    TCase* tc_copy = tcase_create("copy");
    TCase* tc_toframes = tcase_create("toframes");
    TCase* tc_fromframes = tcase_create("fromframes");
    suite_add_tcase(s, tc_init);
    suite_add_tcase(s, tc_set);
    suite_add_tcase(s, tc_cmp);
    suite_add_tcase(s, tc_add);
    suite_add_tcase(s, tc_sub);
    suite_add_tcase(s, tc_copy);
    suite_add_tcase(s, tc_toframes);
    suite_add_tcase(s, tc_fromframes);

    int timeout = 10;
    tcase_set_timeout(tc_init, timeout);
    tcase_set_timeout(tc_set, timeout);
    tcase_set_timeout(tc_cmp, timeout);
    tcase_set_timeout(tc_add, timeout);
    tcase_set_timeout(tc_sub, timeout);
    tcase_set_timeout(tc_copy, timeout);
    tcase_set_timeout(tc_toframes, timeout);
    tcase_set_timeout(tc_fromframes, timeout);

    tcase_add_test(tc_init, init);
    tcase_add_test(tc_set, set);
    tcase_add_test(tc_cmp, cmp);
    tcase_add_test(tc_add, add);
    tcase_add_test(tc_sub, sub);
    tcase_add_test(tc_copy, copy);
    tcase_add_test(tc_toframes, toframes);
    tcase_add_test(tc_fromframes, fromframes);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_init, init_break, SIGABRT);

    tcase_add_test_raise_signal(tc_set, set_break_reltime, SIGABRT);
    tcase_add_test_raise_signal(tc_set, set_break_part1, SIGABRT);
    tcase_add_test_raise_signal(tc_set, set_break_part2, SIGABRT);
    tcase_add_test_raise_signal(tc_set, set_break_part3, SIGABRT);
    tcase_add_test_raise_signal(tc_set, set_break_part4, SIGABRT);

    tcase_add_test_raise_signal(tc_cmp, cmp_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_cmp, cmp_break_null2, SIGABRT);
    tcase_add_test_raise_signal(tc_cmp, cmp_break_inv11, SIGABRT);
    tcase_add_test_raise_signal(tc_cmp, cmp_break_inv12, SIGABRT);
    tcase_add_test_raise_signal(tc_cmp, cmp_break_inv13, SIGABRT);
    tcase_add_test_raise_signal(tc_cmp, cmp_break_inv14, SIGABRT);
    tcase_add_test_raise_signal(tc_cmp, cmp_break_inv21, SIGABRT);
    tcase_add_test_raise_signal(tc_cmp, cmp_break_inv22, SIGABRT);
    tcase_add_test_raise_signal(tc_cmp, cmp_break_inv23, SIGABRT);
    tcase_add_test_raise_signal(tc_cmp, cmp_break_inv24, SIGABRT);

    tcase_add_test_raise_signal(tc_add, add_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_add, add_break_null2, SIGABRT);
    tcase_add_test_raise_signal(tc_add, add_break_null3, SIGABRT);
    tcase_add_test_raise_signal(tc_add, add_break_inv21, SIGABRT);
    tcase_add_test_raise_signal(tc_add, add_break_inv22, SIGABRT);
    tcase_add_test_raise_signal(tc_add, add_break_inv23, SIGABRT);
    tcase_add_test_raise_signal(tc_add, add_break_inv24, SIGABRT);
    tcase_add_test_raise_signal(tc_add, add_break_inv31, SIGABRT);
    tcase_add_test_raise_signal(tc_add, add_break_inv32, SIGABRT);
    tcase_add_test_raise_signal(tc_add, add_break_inv33, SIGABRT);
    tcase_add_test_raise_signal(tc_add, add_break_inv34, SIGABRT);

    tcase_add_test_raise_signal(tc_sub, sub_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_sub, sub_break_null2, SIGABRT);
    tcase_add_test_raise_signal(tc_sub, sub_break_null3, SIGABRT);
    tcase_add_test_raise_signal(tc_sub, sub_break_inv21, SIGABRT);
    tcase_add_test_raise_signal(tc_sub, sub_break_inv22, SIGABRT);
    tcase_add_test_raise_signal(tc_sub, sub_break_inv23, SIGABRT);
    tcase_add_test_raise_signal(tc_sub, sub_break_inv24, SIGABRT);
    tcase_add_test_raise_signal(tc_sub, sub_break_inv31, SIGABRT);
    tcase_add_test_raise_signal(tc_sub, sub_break_inv32, SIGABRT);
    tcase_add_test_raise_signal(tc_sub, sub_break_inv33, SIGABRT);
    tcase_add_test_raise_signal(tc_sub, sub_break_inv34, SIGABRT);

    tcase_add_test_raise_signal(tc_copy, copy_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_copy, copy_break_null2, SIGABRT);
    tcase_add_test_raise_signal(tc_copy, copy_break_inv21, SIGABRT);
    tcase_add_test_raise_signal(tc_copy, copy_break_inv22, SIGABRT);
    tcase_add_test_raise_signal(tc_copy, copy_break_inv23, SIGABRT);
    tcase_add_test_raise_signal(tc_copy, copy_break_inv24, SIGABRT);

    tcase_add_test_raise_signal(tc_toframes, toframes_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_toframes, toframes_break_inv11, SIGABRT);
    tcase_add_test_raise_signal(tc_toframes, toframes_break_inv12, SIGABRT);
    tcase_add_test_raise_signal(tc_toframes, toframes_break_inv13, SIGABRT);
    tcase_add_test_raise_signal(tc_toframes, toframes_break_inv14, SIGABRT);
    tcase_add_test_raise_signal(tc_toframes, toframes_break_inv15, SIGABRT);
    tcase_add_test_raise_signal(tc_toframes, toframes_break_inv16, SIGABRT);
    tcase_add_test_raise_signal(tc_toframes, toframes_break_inv21, SIGABRT);
    tcase_add_test_raise_signal(tc_toframes, toframes_break_inv22, SIGABRT);
    tcase_add_test_raise_signal(tc_toframes, toframes_break_inv31, SIGABRT);

    tcase_add_test_raise_signal(tc_fromframes, fromframes_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_fromframes, fromframes_break_inv_tempo1, SIGABRT);
    tcase_add_test_raise_signal(tc_fromframes, fromframes_break_inv_tempo2, SIGABRT);
    tcase_add_test_raise_signal(tc_fromframes, fromframes_break_inv_freq, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Reltime_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    if (fail_count > 0)
    {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}


