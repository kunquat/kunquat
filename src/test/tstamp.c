

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


#include <test_common.h>

#include <kunquat/testing.h>
#include <mathnum/Tstamp.h>

#include <assert.h>
#include <float.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


static void silent_assert(void)
{
    kqt_suppress_assert_messages();
}


START_TEST (init)
{
    Tstamp rel;
    Tstamp* rp = Tstamp_init(&rel);
    fail_unless(rp == &rel,
            "Tstamp_init() returned %p instead of %p.", rp, &rel);
    fail_unless(rp->beats == 0,
            "Tstamp_init() set beats to %lld instead of 0.", (long long)rp->beats);
    fail_unless(rp->rem == 0,
            "Tstamp_init() set remainder to %ld instead of 0.", (long)rp->rem);
}
END_TEST

#ifndef NDEBUG
START_TEST (init_break)
{
    Tstamp_init(NULL);
}
END_TEST
#endif

START_TEST (set)
{
    int64_t beat_values[] = { INT64_MIN, INT64_MIN + 1, -1, 0, 1, INT64_MAX - 1, INT64_MAX };
    int32_t rem_values[] = { 0, 1, KQT_TSTAMP_BEAT - 1 };
    int32_t all_rems[] = { INT32_MIN, INT32_MIN + 1, -1, 0, 1,
            KQT_TSTAMP_BEAT - 1, KQT_TSTAMP_BEAT, INT32_MAX - 1, INT32_MAX };
    for (size_t i = 0; i < sizeof(beat_values) / sizeof(int64_t); ++i)
    {
        for (size_t k = 0; k < sizeof(rem_values) / sizeof(int32_t); ++k)
        {
            for (size_t l = 0; l < sizeof(all_rems) / sizeof(int32_t); ++l)
            {
                Tstamp* r = Tstamp_init(&(Tstamp){ .beats = 0 });
                r->rem = all_rems[l];
                Tstamp* s = Tstamp_set(r, beat_values[i], rem_values[k]);
                fail_unless(s == r,
                        "Tstamp_set() returned %p instead of %p.", s, r);
                fail_unless(s->beats == beat_values[i],
                        "Tstamp_set() set beats to %lld instead of %lld.",
                        (long long)s->beats, (long long)beat_values[i]);
                fail_unless(s->rem == rem_values[k],
                        "Tstamp_set() set remainder to %lld instead of %lld.",
                        (long long)s->rem, (long long)rem_values[k]);
            }
        }
    }
}
END_TEST

#ifndef NDEBUG
START_TEST (set_break_tstamp)
{
    Tstamp_set(NULL, 0, 0);
}
END_TEST

START_TEST (set_break_rem1)
{
    Tstamp_set(&(Tstamp){ .beats = 0 }, 0, INT32_MIN);
}
END_TEST

START_TEST (set_break_rem2)
{
    Tstamp_set(&(Tstamp){ .beats = 0 }, 0, -1);
}
END_TEST

START_TEST (set_break_rem3)
{
    Tstamp_set(&(Tstamp){ .beats = 0 }, 0, KQT_TSTAMP_BEAT);
}
END_TEST

START_TEST (set_break_rem4)
{
    Tstamp_set(&(Tstamp){ .beats = 0 }, 0, INT32_MAX);
}
END_TEST
#endif

START_TEST (cmp)
{
    Tstamp* r1 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp* r2 = Tstamp_init(&(Tstamp){ .beats = 0 });
    int res = 0;
#define CMPTEXT(c) ((c) < 0 ? "smaller" : ((c) > 0 ? "greater" : "equal"))

    // beats and remainders equal
    Tstamp_set(r1, INT64_MIN, 0);
    Tstamp_set(r2, INT64_MIN, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MIN, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, INT64_MIN, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));

    Tstamp_set(r1, INT64_MIN + 1, 0);
    Tstamp_set(r2, INT64_MIN + 1, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MIN + 1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, INT64_MIN + 1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));

    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, -1, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));
    Tstamp_set(r1, -1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, -1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));

    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, 0, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));
    Tstamp_set(r1, 0, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, 0, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));

    Tstamp_set(r1, 1, 0);
    Tstamp_set(r2, 1, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));
    Tstamp_set(r1, 1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, 1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));

    Tstamp_set(r1, INT64_MAX - 1, 0);
    Tstamp_set(r2, INT64_MAX - 1, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MAX - 1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, INT64_MAX - 1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));

    Tstamp_set(r1, INT64_MAX, 0);
    Tstamp_set(r2, INT64_MAX, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MAX, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, INT64_MAX, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res == 0,
            "Tstamp_cmp() returned %s instead of equal.", CMPTEXT(res));

    // beats equal and remainders unequal
    Tstamp_set(r1, INT64_MIN, 0);
    Tstamp_set(r2, INT64_MIN, 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MIN, 0);
    Tstamp_set(r2, INT64_MIN, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MIN, KQT_TSTAMP_BEAT - 2);
    Tstamp_set(r2, INT64_MIN, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, -1, 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, -1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, -1, KQT_TSTAMP_BEAT - 2);
    Tstamp_set(r2, -1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, 0, 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, 0, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, 0, KQT_TSTAMP_BEAT - 2);
    Tstamp_set(r2, 0, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, 1, 0);
    Tstamp_set(r2, 1, 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, 1, 0);
    Tstamp_set(r2, 1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, 1, KQT_TSTAMP_BEAT - 2);
    Tstamp_set(r2, 1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, INT64_MAX, 0);
    Tstamp_set(r2, INT64_MAX, 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MAX, 0);
    Tstamp_set(r2, INT64_MAX, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MAX, KQT_TSTAMP_BEAT - 2);
    Tstamp_set(r2, INT64_MAX, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    // beats unequal and remainders equal
    Tstamp_set(r1, INT64_MIN, 0);
    Tstamp_set(r2, INT64_MIN + 1, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MIN, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, INT64_MIN + 1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, 0, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, -1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, 0, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, 1, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, 0, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, 1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, INT64_MAX - 1, 0);
    Tstamp_set(r2, INT64_MAX, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MAX - 1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, INT64_MAX, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, INT64_MIN, 0);
    Tstamp_set(r2, INT64_MAX, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MIN, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, INT64_MAX, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    // beats and remainders unequal
    Tstamp_set(r1, INT64_MIN, 0);
    Tstamp_set(r2, INT64_MIN + 1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MIN, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, INT64_MIN + 1, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, 0, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, -1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, 0, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, 1, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, 0, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, 1, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, INT64_MAX - 1, 0);
    Tstamp_set(r2, INT64_MAX, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MAX - 1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, INT64_MAX, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));

    Tstamp_set(r1, INT64_MIN, 0);
    Tstamp_set(r2, INT64_MAX, KQT_TSTAMP_BEAT - 1);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
    Tstamp_set(r1, INT64_MIN, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, INT64_MAX, 0);
    res = Tstamp_cmp(r1, r2);
    fail_unless(res < 0,
            "Tstamp_cmp() returned %s instead of smaller.", CMPTEXT(res));
    res = Tstamp_cmp(r2, r1);
    fail_unless(res > 0,
            "Tstamp_cmp() returned %s instead of greater.", CMPTEXT(res));
}
END_TEST

#ifndef NDEBUG
START_TEST (cmp_break_null1)
{
    Tstamp_cmp(NULL, Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (cmp_break_null2)
{
    Tstamp_cmp(Tstamp_init(&(Tstamp){ .beats = 0 }), NULL);
}
END_TEST

START_TEST (cmp_break_inv11)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MIN };
    Tstamp_cmp(br, Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (cmp_break_inv12)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = -1 };
    Tstamp_cmp(br, Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (cmp_break_inv13)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = KQT_TSTAMP_BEAT };
    Tstamp_cmp(br, Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (cmp_break_inv14)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MAX };
    Tstamp_cmp(br, Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (cmp_break_inv21)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MIN };
    Tstamp_cmp(Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST

START_TEST (cmp_break_inv22)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = -1 };
    Tstamp_cmp(Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST

START_TEST (cmp_break_inv23)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = KQT_TSTAMP_BEAT };
    Tstamp_cmp(Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST

START_TEST (cmp_break_inv24)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MAX };
    Tstamp_cmp(Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST
#endif


START_TEST (add)
{
    Tstamp* ret = NULL;
    Tstamp* res = Tstamp_init(TSTAMP_AUTO);
    res->rem = -1;
    Tstamp* r1 = Tstamp_init(TSTAMP_AUTO);
    Tstamp* r2 = Tstamp_init(TSTAMP_AUTO);
    Tstamp* exp = Tstamp_init(TSTAMP_AUTO);

    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, -1, 1);
    Tstamp_set(exp, -2, 1);
    ret = Tstamp_add(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_add(res, r2, r1);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, 0, 1);
    Tstamp_set(exp, -1, 1);
    ret = Tstamp_add(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_add(res, r2, r1);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, -1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, 0, 1);
    Tstamp_set(exp, 0, 0);
    ret = Tstamp_add(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_add(res, r2, r1);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, 0, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, 0, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(exp, 1, KQT_TSTAMP_BEAT - 2);
    ret = Tstamp_add(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_add(res, r2, r1);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, 0, 0);
    Tstamp_set(exp, -1, 0);
    ret = Tstamp_add(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_add(res, r2, r1);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, 0, 0);
    Tstamp_set(exp, 0, 0);
    ret = Tstamp_add(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_add(res, r2, r1);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, 1, 0);
    Tstamp_set(r2, 0, 0);
    Tstamp_set(exp, 1, 0);
    ret = Tstamp_add(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_add(res, r2, r1);
    fail_unless(ret == res,
            "Tstamp_add() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_add() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    // Assignment version
    Tstamp_set(r1, 2, 0);
    Tstamp_set(r2, 3, 0);
    Tstamp_set(exp, 5, 0);
    ret = Tstamp_adda(r1, r2);
    fail_unless(ret == r1,
            "Tstamp_adda() returned %p instead of %p.", ret, r1);
    fail_unless(Tstamp_cmp(r1, exp) == 0,
            "Tstamp_adda() returned " PRIts " instead of " PRIts,
            PRIVALts(*r1), PRIVALts(*exp));
}
END_TEST

#ifndef NDEBUG
START_TEST (add_break_null1)
{
    Tstamp* r1 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp* r2 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp_add(NULL, r1, r2);
}
END_TEST

START_TEST (add_break_null2)
{
    Tstamp* r1 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp* r2 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp_add(r1, NULL, r2);
}
END_TEST

START_TEST (add_break_null3)
{
    Tstamp* r1 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp* r2 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp_add(r1, r2, NULL);
}
END_TEST

START_TEST (add_break_inv21)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MIN };
    Tstamp_add(Tstamp_init(&(Tstamp){ .beats = 0 }), br,
            Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (add_break_inv22)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = -1 };
    Tstamp_add(Tstamp_init(&(Tstamp){ .beats = 0 }), br,
            Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (add_break_inv23)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = KQT_TSTAMP_BEAT };
    Tstamp_add(Tstamp_init(&(Tstamp){ .beats = 0 }), br,
            Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (add_break_inv24)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MAX };
    Tstamp_add(Tstamp_init(&(Tstamp){ .beats = 0 }), br,
            Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (add_break_inv31)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MIN };
    Tstamp_add(Tstamp_init(&(Tstamp){ .beats = 0 }),
            Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST

START_TEST (add_break_inv32)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = -1 };
    Tstamp_add(Tstamp_init(&(Tstamp){ .beats = 0 }),
            Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST

START_TEST (add_break_inv33)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = KQT_TSTAMP_BEAT };
    Tstamp_add(Tstamp_init(&(Tstamp){ .beats = 0 }),
            Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST

START_TEST (add_break_inv34)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MAX };
    Tstamp_add(Tstamp_init(&(Tstamp){ .beats = 0 }),
            Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST
#endif


START_TEST (sub)
{
    Tstamp* ret = NULL;
    Tstamp* res = Tstamp_init(TSTAMP_AUTO);
    res->rem = -1;
    Tstamp* r1 = Tstamp_init(TSTAMP_AUTO);
    Tstamp* r2 = Tstamp_init(TSTAMP_AUTO);
    Tstamp* exp = Tstamp_init(TSTAMP_AUTO);

    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, -1, 1);
    Tstamp_set(exp, -1, KQT_TSTAMP_BEAT - 1);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, 0, 1);
    Tstamp_set(exp, -2, KQT_TSTAMP_BEAT - 1);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, -1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, 0, 1);
    Tstamp_set(exp, -1, KQT_TSTAMP_BEAT - 2);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, 0, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(r2, 0, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(exp, 0, 0);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, -1, 0);
    Tstamp_set(r2, 0, 0);
    Tstamp_set(exp, -1, 0);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, 0, 0);
    Tstamp_set(exp, 0, 0);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, 1, 0);
    Tstamp_set(r2, 0, 0);
    Tstamp_set(exp, 1, 0);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, -1, 0);
    Tstamp_set(exp, 1, 0);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, 1, 0);
    Tstamp_set(exp, -1, 0);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, 0, 1);
    Tstamp_set(exp, -1, KQT_TSTAMP_BEAT - 1);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(r1, 0, 0);
    Tstamp_set(r2, -1, KQT_TSTAMP_BEAT - 1);
    Tstamp_set(exp, 0, 1);
    ret = Tstamp_sub(res, r1, r2);
    fail_unless(ret == res,
            "Tstamp_sub() returned %p instead of %p.", ret, res);
    fail_unless(Tstamp_cmp(res, exp) == 0,
            "Tstamp_sub() returned %lld:%ld (expected %lld:%ld).",
            (long long)res->beats, (long)res->rem,
            (long long)exp->beats, (long)exp->rem);

    // Assignment version
    Tstamp_set(r1, 3, 0);
    Tstamp_set(r2, 2, 0);
    Tstamp_set(exp, 1, 0);
    ret = Tstamp_suba(r1, r2);
    fail_unless(ret == r1,
            "Tstamp_adda() returned %p instead of %p.", ret, r1);
    fail_unless(Tstamp_cmp(r1, exp) == 0,
            "Tstamp_adda() returned " PRIts " instead of " PRIts,
            PRIVALts(*r1), PRIVALts(*exp));
}
END_TEST

#ifndef NDEBUG
START_TEST (sub_break_null1)
{
    Tstamp* r1 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp* r2 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp_sub(NULL, r1, r2);
}
END_TEST

START_TEST (sub_break_null2)
{
    Tstamp* r1 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp* r2 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp_sub(r1, NULL, r2);
}
END_TEST

START_TEST (sub_break_null3)
{
    Tstamp* r1 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp* r2 = Tstamp_init(&(Tstamp){ .beats = 0 });
    Tstamp_sub(r1, r2, NULL);
}
END_TEST

START_TEST (sub_break_inv21)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MIN };
    Tstamp_sub(Tstamp_init(&(Tstamp){ .beats = 0 }), br,
            Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (sub_break_inv22)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = -1 };
    Tstamp_sub(Tstamp_init(&(Tstamp){ .beats = 0 }), br,
            Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (sub_break_inv23)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = KQT_TSTAMP_BEAT };
    Tstamp_sub(Tstamp_init(&(Tstamp){ .beats = 0 }), br,
            Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (sub_break_inv24)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MAX };
    Tstamp_sub(Tstamp_init(&(Tstamp){ .beats = 0 }), br,
            Tstamp_init(&(Tstamp){ .beats = 0 }));
}
END_TEST

START_TEST (sub_break_inv31)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MIN };
    Tstamp_sub(Tstamp_init(&(Tstamp){ .beats = 0 }),
            Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST

START_TEST (sub_break_inv32)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = -1 };
    Tstamp_sub(Tstamp_init(&(Tstamp){ .beats = 0 }),
            Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST

START_TEST (sub_break_inv33)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = KQT_TSTAMP_BEAT };
    Tstamp_sub(Tstamp_init(&(Tstamp){ .beats = 0 }),
            Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST

START_TEST (sub_break_inv34)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MAX };
    Tstamp_sub(Tstamp_init(&(Tstamp){ .beats = 0 }),
            Tstamp_init(&(Tstamp){ .beats = 0 }), br);
}
END_TEST
#endif


START_TEST (copy)
{
    Tstamp* ret = NULL;
    Tstamp* src = Tstamp_init(TSTAMP_AUTO);
    Tstamp* dest = Tstamp_init(TSTAMP_AUTO);

    Tstamp_set(src, INT64_MAX, KQT_TSTAMP_BEAT - 1);
    ret = Tstamp_copy(dest, src);
    fail_unless(ret == dest,
            "Tstamp_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Tstamp_cmp(dest, src) == 0,
            "Tstamp_copy() didn't produce a copy equal to the original.");

    Tstamp_set(src, INT64_MAX, 0);
    ret = Tstamp_copy(dest, src);
    fail_unless(ret == dest,
            "Tstamp_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Tstamp_cmp(dest, src) == 0,
            "Tstamp_copy() didn't produce a copy equal to the original.");

    Tstamp_set(src, 1, 0);
    ret = Tstamp_copy(dest, src);
    fail_unless(ret == dest,
            "Tstamp_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Tstamp_cmp(dest, src) == 0,
            "Tstamp_copy() didn't produce a copy equal to the original.");

    Tstamp_set(src, 0, KQT_TSTAMP_BEAT - 1);
    ret = Tstamp_copy(dest, src);
    fail_unless(ret == dest,
            "Tstamp_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Tstamp_cmp(dest, src) == 0,
            "Tstamp_copy() didn't produce a copy equal to the original.");

    Tstamp_set(src, 0, 0);
    ret = Tstamp_copy(dest, src);
    fail_unless(ret == dest,
            "Tstamp_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Tstamp_cmp(dest, src) == 0,
            "Tstamp_copy() didn't produce a copy equal to the original.");

    Tstamp_set(src, -1, KQT_TSTAMP_BEAT - 1);
    ret = Tstamp_copy(dest, src);
    fail_unless(ret == dest,
            "Tstamp_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Tstamp_cmp(dest, src) == 0,
            "Tstamp_copy() didn't produce a copy equal to the original.");

    Tstamp_set(src, -1, 1);
    ret = Tstamp_copy(dest, src);
    fail_unless(ret == dest,
            "Tstamp_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Tstamp_cmp(dest, src) == 0,
            "Tstamp_copy() didn't produce a copy equal to the original.");

    Tstamp_set(src, INT64_MIN, KQT_TSTAMP_BEAT - 1);
    ret = Tstamp_copy(dest, src);
    fail_unless(ret == dest,
            "Tstamp_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Tstamp_cmp(dest, src) == 0,
            "Tstamp_copy() didn't produce a copy equal to the original.");

    Tstamp_set(src, INT64_MIN, 0);
    ret = Tstamp_copy(dest, src);
    fail_unless(ret == dest,
            "Tstamp_copy() returned %p instead of %p.", ret, dest);
    fail_unless(Tstamp_cmp(dest, src) == 0,
            "Tstamp_copy() didn't produce a copy equal to the original.");
}
END_TEST

#ifndef NDEBUG
START_TEST (copy_break_null1)
{
    Tstamp_copy(NULL, Tstamp_init(TSTAMP_AUTO));
}
END_TEST

START_TEST (copy_break_null2)
{
    Tstamp_copy(Tstamp_init(TSTAMP_AUTO), NULL);
}
END_TEST

START_TEST (copy_break_inv21)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MIN };
    Tstamp_copy(Tstamp_init(TSTAMP_AUTO), br);
}
END_TEST

START_TEST (copy_break_inv22)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = -1 };
    Tstamp_copy(Tstamp_init(TSTAMP_AUTO), br);
}
END_TEST

START_TEST (copy_break_inv23)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = KQT_TSTAMP_BEAT };
    Tstamp_copy(Tstamp_init(TSTAMP_AUTO), br);
}
END_TEST

START_TEST (copy_break_inv24)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MAX };
    Tstamp_copy(Tstamp_init(TSTAMP_AUTO), br);
}
END_TEST
#endif


START_TEST (toframes)
{
    Tstamp* r = Tstamp_init(TSTAMP_AUTO);
    double res = 0;
    res = Tstamp_toframes(r, DBL_MIN, 1);
    fail_unless(res == 0,
            "Tstamp_toframes() returned %.4f instead of 0.", res);
    res = Tstamp_toframes(r, DBL_MIN, INT32_MAX);
    fail_unless(res == 0,
            "Tstamp_toframes() returned %.4f instead of 0.", res);
    res = Tstamp_toframes(r, DBL_MAX, 1);
    fail_unless(res == 0,
            "Tstamp_toframes() returned %.4f instead of 0.", res);
    res = Tstamp_toframes(r, DBL_MAX, INT32_MAX);
    fail_unless(res == 0,
            "Tstamp_toframes() returned %.4f instead of 0.", res);

    Tstamp_set(r, 1, 0);
    res = Tstamp_toframes(r, 60, 44100);
    fail_unless(res == 44100,
            "Tstamp_toframes() returned %.4f instead of 44100.", res);
    res = Tstamp_toframes(r, 120, 44100);
    fail_unless(res == 22050,
            "Tstamp_toframes() returned %.4f instead of 22050.", res);
    res = Tstamp_toframes(r, 60, 96000);
    fail_unless(res == 96000,
            "Tstamp_toframes() returned %.4f instead of 96000.", res);

    Tstamp_set(r, 0, KQT_TSTAMP_BEAT / 2);
    res = Tstamp_toframes(r, 60, 44100);
    fail_unless(res == 22050,
            "Tstamp_toframes() returned %.4f instead of 22050.", res);
    res = Tstamp_toframes(r, 120, 44100);
    fail_unless(res == 11025,
            "Tstamp_toframes() returned %.4f instead of 11025.", res);
    res = Tstamp_toframes(r, 60, 96000);
    fail_unless(res == 48000,
            "Tstamp_toframes() returned %.4f instead of 48000.", res);

    Tstamp_set(r, 1, KQT_TSTAMP_BEAT / 2);
    res = Tstamp_toframes(r, 60, 44100);
    fail_unless(res == 66150,
            "Tstamp_toframes() returned %.4f instead of 66150.", res);
    res = Tstamp_toframes(r, 120, 44100);
    fail_unless(res == 33075,
            "Tstamp_toframes() returned %.4f instead of 33075.", res);
    res = Tstamp_toframes(r, 60, 96000);
    fail_unless(res == 144000,
            "Tstamp_toframes() returned %.4f instead of 144000.", res);
}
END_TEST

#ifndef NDEBUG
START_TEST (toframes_break_null1)
{
    Tstamp_toframes(NULL, 1, 1);
}
END_TEST

START_TEST (toframes_break_inv11)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MIN };
    Tstamp_toframes(br, 1, 1);
}
END_TEST

START_TEST (toframes_break_inv12)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = -1 };
    Tstamp_toframes(br, 1, 1);
}
END_TEST

START_TEST (toframes_break_inv13)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = KQT_TSTAMP_BEAT };
    Tstamp_toframes(br, 1, 1);
}
END_TEST

START_TEST (toframes_break_inv14)
{
    Tstamp* br = &(Tstamp){ .beats = 0, .rem = INT32_MAX };
    Tstamp_toframes(br, 1, 1);
}
END_TEST

START_TEST (toframes_break_inv15)
{
    Tstamp_toframes(Tstamp_set(TSTAMP_AUTO, INT64_MIN, 0), 1, 1);
}
END_TEST

START_TEST (toframes_break_inv16)
{
    Tstamp_toframes(Tstamp_set(TSTAMP_AUTO, -1, KQT_TSTAMP_BEAT - 1), 1, 1);
}
END_TEST

START_TEST (toframes_break_inv21)
{
    Tstamp_toframes(Tstamp_init(TSTAMP_AUTO), -DBL_MAX, 1);
}
END_TEST

START_TEST (toframes_break_inv22)
{
    Tstamp_toframes(Tstamp_init(TSTAMP_AUTO), 0, 1);
}
END_TEST

START_TEST (toframes_break_inv31)
{
    Tstamp_toframes(Tstamp_init(TSTAMP_AUTO), 1, 0);
}
END_TEST
#endif


START_TEST (fromframes)
{
    Tstamp* r = &(Tstamp){ .beats = 0, .rem = -1 };
    Tstamp* ret = NULL;
    Tstamp* exp = Tstamp_init(TSTAMP_AUTO);

    ret = Tstamp_fromframes(r, 0, DBL_MIN, 1);
    fail_unless(ret == r,
            "Tstamp_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Tstamp_cmp(r, exp) == 0,
            "Tstamp_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_fromframes(r, 0, DBL_MIN, INT32_MAX);
    fail_unless(ret == r,
            "Tstamp_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Tstamp_cmp(r, exp) == 0,
            "Tstamp_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_fromframes(r, 0, DBL_MAX, INT32_MAX);
    fail_unless(ret == r,
            "Tstamp_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Tstamp_cmp(r, exp) == 0,
            "Tstamp_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_fromframes(r, 0, DBL_MAX, INT32_MAX);
    fail_unless(ret == r,
            "Tstamp_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Tstamp_cmp(r, exp) == 0,
            "Tstamp_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(exp, 1, 0);
    ret = Tstamp_fromframes(r, 44100, 60, 44100);
    fail_unless(ret == r,
            "Tstamp_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Tstamp_cmp(r, exp) == 0,
            "Tstamp_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_fromframes(r, 48000, 120, 96000);
    fail_unless(ret == r,
            "Tstamp_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Tstamp_cmp(r, exp) == 0,
            "Tstamp_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(exp, 0, KQT_TSTAMP_BEAT / 2);
    ret = Tstamp_fromframes(r, 22050, 60, 44100);
    fail_unless(ret == r,
            "Tstamp_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Tstamp_cmp(r, exp) == 0,
            "Tstamp_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_fromframes(r, 24000, 120, 96000);
    fail_unless(ret == r,
            "Tstamp_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Tstamp_cmp(r, exp) == 0,
            "Tstamp_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);

    Tstamp_set(exp, 1, KQT_TSTAMP_BEAT / 2);
    ret = Tstamp_fromframes(r, 66150, 60, 44100);
    fail_unless(ret == r,
            "Tstamp_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Tstamp_cmp(r, exp) == 0,
            "Tstamp_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
    ret = Tstamp_fromframes(r, 72000, 120, 96000);
    fail_unless(ret == r,
            "Tstamp_fromframes() returned %p instead of %p.", ret, r);
    fail_unless(Tstamp_cmp(r, exp) == 0,
            "Tstamp_fromframes() returned %lld:%ld instead of %lld:%ld.",
            (long long)r->beats, (long)r->rem,
            (long long)exp->beats, (long)exp->rem);
}
END_TEST

#ifndef NDEBUG
START_TEST (fromframes_break_null1)
{
    Tstamp_fromframes(NULL, 0, 1, 1);
}
END_TEST

START_TEST (fromframes_break_inv_tempo1)
{
    Tstamp_fromframes(Tstamp_init(TSTAMP_AUTO), 0, -DBL_MAX, 1);
}
END_TEST

START_TEST (fromframes_break_inv_tempo2)
{
    Tstamp_fromframes(Tstamp_init(TSTAMP_AUTO), 0, 0, 1);
}
END_TEST

START_TEST (fromframes_break_inv_freq)
{
    Tstamp_fromframes(Tstamp_init(TSTAMP_AUTO), 0, 1, 0);
}
END_TEST
#endif


static Suite* Tstamp_suite(void)
{
    Suite* s = suite_create("Tstamp");
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

    const int timeout = DEFAULT_TIMEOUT;
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
    TCase* tc_asserts = tcase_create("asserts");
    suite_add_tcase(s, tc_asserts);
    const int assert_timeout = 10;
    tcase_set_timeout(tc_asserts, assert_timeout);
    tcase_add_checked_fixture(tc_asserts, silent_assert, NULL);

    tcase_add_test_raise_signal(tc_asserts, init_break, SIGABRT);

    tcase_add_test_raise_signal(tc_asserts, set_break_tstamp, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, set_break_rem1, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, set_break_rem2, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, set_break_rem3, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, set_break_rem4, SIGABRT);

    tcase_add_test_raise_signal(tc_asserts, cmp_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, cmp_break_null2, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, cmp_break_inv11, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, cmp_break_inv12, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, cmp_break_inv13, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, cmp_break_inv14, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, cmp_break_inv21, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, cmp_break_inv22, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, cmp_break_inv23, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, cmp_break_inv24, SIGABRT);

    tcase_add_test_raise_signal(tc_asserts, add_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, add_break_null2, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, add_break_null3, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, add_break_inv21, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, add_break_inv22, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, add_break_inv23, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, add_break_inv24, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, add_break_inv31, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, add_break_inv32, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, add_break_inv33, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, add_break_inv34, SIGABRT);

    tcase_add_test_raise_signal(tc_asserts, sub_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, sub_break_null2, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, sub_break_null3, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, sub_break_inv21, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, sub_break_inv22, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, sub_break_inv23, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, sub_break_inv24, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, sub_break_inv31, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, sub_break_inv32, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, sub_break_inv33, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, sub_break_inv34, SIGABRT);

    tcase_add_test_raise_signal(tc_asserts, copy_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, copy_break_null2, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, copy_break_inv21, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, copy_break_inv22, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, copy_break_inv23, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, copy_break_inv24, SIGABRT);

    tcase_add_test_raise_signal(tc_asserts, toframes_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, toframes_break_inv11, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, toframes_break_inv12, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, toframes_break_inv13, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, toframes_break_inv14, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, toframes_break_inv15, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, toframes_break_inv16, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, toframes_break_inv21, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, toframes_break_inv22, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, toframes_break_inv31, SIGABRT);

    tcase_add_test_raise_signal(tc_asserts, fromframes_break_null1, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, fromframes_break_inv_tempo1, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, fromframes_break_inv_tempo2, SIGABRT);
    tcase_add_test_raise_signal(tc_asserts, fromframes_break_inv_freq, SIGABRT);
#endif

    return s;
}


int main(void)
{
    Suite* suite = Tstamp_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


