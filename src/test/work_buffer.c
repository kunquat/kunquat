

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <handle_utils.h>
#include <test_common.h>

#include <player/Work_buffer.h>

#include <assert.h>
#include <stdlib.h>


#define BUFFER_SIZE 128

#define SUB_COUNTS 3
static const int sub_counts[SUB_COUNTS] = { 1, 2, 4 };


static void fill_buffer_area(float* area_start, int sub_count)
{
    for (int i = 0; i < BUFFER_SIZE * sub_count; ++i)
    {
        const int sub_index = i % sub_count;
        area_start[i] = (float)(sub_index + (i / sub_count) * 10);
    }

    return;
}


START_TEST(Initial_work_buffer_contains_silence)
{
    const int sub_count = sub_counts[_i];

    Work_buffer* buffer = new_Work_buffer(BUFFER_SIZE, sub_count);
    assert(buffer != NULL);

    const float* contents = Work_buffer_get_contents(buffer, 0);

    float expected[BUFFER_SIZE * WORK_BUFFER_SUB_COUNT_MAX] = { 0 };

    check_buffers_equal(expected, contents, BUFFER_SIZE * sub_count, 0);

    del_Work_buffer(buffer);
}
END_TEST


START_TEST(Work_buffer_gives_correct_subareas)
{
    const int sub_count = sub_counts[_i];

    Work_buffer* buffer = new_Work_buffer(BUFFER_SIZE, sub_count);
    assert(buffer != NULL);

    const int stride = Work_buffer_get_stride(buffer);
    fail_if(stride != sub_count, "Incorrect stride %d, expected %d", stride, sub_count);

    const float* contents = Work_buffer_get_contents(buffer, 0);

    for (int sub_index = 0; sub_index < sub_count; ++sub_index)
    {
        float* write_pos = Work_buffer_get_contents_mut(buffer, sub_index);
        const float* expected_start = contents + sub_index;
        fail_if(write_pos != expected_start,
                "Incorrect buffer area start at sub index %d: offset by %d",
                sub_index, (int)(expected_start - write_pos));

        for (int i = 0; i < BUFFER_SIZE; ++i)
        {
            *write_pos = (float)(sub_index + i * 10);
            write_pos += stride;
        }
    }

    float expected[BUFFER_SIZE * WORK_BUFFER_SUB_COUNT_MAX] = { 0 };
    fill_buffer_area(expected, sub_count);

    check_buffers_equal(expected, contents, BUFFER_SIZE * sub_count, 0);

    del_Work_buffer(buffer);
}
END_TEST


START_TEST(Work_buffer_clears_correct_subarea)
{
    const int sub_count = sub_counts[_i];

    Work_buffer* buffer = new_Work_buffer(BUFFER_SIZE, sub_count);
    assert(buffer != NULL);

    float expected[BUFFER_SIZE * WORK_BUFFER_SUB_COUNT_MAX] = { 0 };

    for (int sub_index = 0; sub_index < sub_count; ++sub_index)
    {
        float* contents = Work_buffer_get_contents_mut(buffer, 0);
        fill_buffer_area(contents, sub_count);
        Work_buffer_clear(buffer, sub_index, 0, Work_buffer_get_size(buffer));

        fill_buffer_area(expected, sub_count);
        float* expected_zero = expected + sub_index;
        for (int i = 0; i < BUFFER_SIZE; ++i)
        {
            *expected_zero = 0;
            expected_zero += sub_count;
        }

        check_buffers_equal(expected, contents, BUFFER_SIZE * sub_count, 0);
    }

    del_Work_buffer(buffer);
}
END_TEST


START_TEST(Work_buffer_copies_correct_subarea)
{
    const int sub_count = sub_counts[_i];

    Work_buffer* dest = new_Work_buffer(BUFFER_SIZE, sub_count);
    Work_buffer* src = new_Work_buffer(BUFFER_SIZE, sub_count);
    assert(dest != NULL);
    assert(src != NULL);

    float expected[BUFFER_SIZE * WORK_BUFFER_SUB_COUNT_MAX] = { 0 };

    float* src_contents = Work_buffer_get_contents_mut(src, 0);
    fill_buffer_area(src_contents, sub_count);

    const float* dest_contents = Work_buffer_get_contents(dest, 0);

    for (int sub_index = 0; sub_index < sub_count; ++sub_index)
    {
        for (int i = 0; i < sub_count; ++i)
            Work_buffer_clear(dest, i, 0, Work_buffer_get_size(dest));

        Work_buffer_copy(
                dest, sub_index, src, sub_index, 0, Work_buffer_get_size(dest));

        fill_buffer_area(expected, sub_count);
        for (int i = 0; i < BUFFER_SIZE * sub_count; ++i)
        {
            const int sub = i % sub_count;
            if (sub != sub_index)
                expected[i] = 0;
        }

        check_buffers_equal(expected, dest_contents, BUFFER_SIZE * sub_count, 0);
    }

    del_Work_buffer(dest);
    del_Work_buffer(src);
}
END_TEST


START_TEST(Work_buffer_mixes_correct_subarea)
{
    const int sub_count = sub_counts[_i];

    Work_buffer* dest = new_Work_buffer(BUFFER_SIZE, sub_count);
    Work_buffer* src = new_Work_buffer(BUFFER_SIZE, sub_count);
    assert(dest != NULL);
    assert(src != NULL);

    float expected[BUFFER_SIZE * WORK_BUFFER_SUB_COUNT_MAX] = { 0 };

    float* src_contents = Work_buffer_get_contents_mut(src, 0);
    fill_buffer_area(src_contents, sub_count);

    float* dest_contents = Work_buffer_get_contents_mut(dest, 0);

    for (int sub_index = 0; sub_index < sub_count; ++sub_index)
    {
        for (int i = 0; i < BUFFER_SIZE * sub_count; ++i)
            dest_contents[i] = 10000;

        Work_buffer_mix(
                dest, sub_index, src, sub_index, 0, Work_buffer_get_size(dest));

        fill_buffer_area(expected, sub_count);
        for (int i = 0; i < BUFFER_SIZE * sub_count; ++i)
        {
            const int sub = i % sub_count;
            if (sub == sub_index)
                expected[i] += 10000;
            else
                expected[i] = 10000;
        }

        check_buffers_equal(expected, dest_contents, BUFFER_SIZE * sub_count, 0);
    }

    del_Work_buffer(dest);
    del_Work_buffer(src);
}
END_TEST


Suite* Work_buffer_suite(void)
{
    Suite* s = suite_create("Work buffer");

    const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_subareas = tcase_create("subareas");
    suite_add_tcase(s, tc_subareas);
    tcase_set_timeout(tc_subareas, timeout);

    tcase_add_loop_test(
            tc_subareas, Initial_work_buffer_contains_silence, 0, SUB_COUNTS);
    tcase_add_loop_test(tc_subareas, Work_buffer_gives_correct_subareas, 0, SUB_COUNTS);
    tcase_add_loop_test(tc_subareas, Work_buffer_clears_correct_subarea, 0, SUB_COUNTS);
    tcase_add_loop_test(tc_subareas, Work_buffer_copies_correct_subarea, 0, SUB_COUNTS);
    tcase_add_loop_test(tc_subareas, Work_buffer_mixes_correct_subarea, 0, SUB_COUNTS);

    return s;
}


int main(void)
{
    Suite* suite = Work_buffer_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


