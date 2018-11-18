

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2018
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

#include <kunquat/Handle.h>
#include <kunquat/testing.h>
#include <memory.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#ifdef KQT_LONG_TESTS
START_TEST(Out_of_memory_at_handle_creation_fails_cleanly)
{
    assert(handle == 0);

    // Get the number of memory allocations required by a successfull call
    handle = kqt_new_Handle();
    if (handle == 0)
    {
        fail("Normal handle creation failed");
        abort();
    }

    const long alloc_count = kqt_get_memory_alloc_count();
    fail_if(alloc_count <= 0,
            "kqt_new_Handle did not allocate memory");
    fail_if(alloc_count >= 65536,
            "kqt_new_Handle made too many memory allocations (%ld)",
            alloc_count);

    kqt_del_Handle(handle);
    handle = 0;

    // Test errors at every memory allocation point
    for (long i = 0; i < alloc_count; ++i)
    {
        //fprintf(stderr, "%ld\n", i);

        kqt_fake_out_of_memory(i);

        handle = kqt_new_Handle();
        fail_if(handle != 0,
                "kqt_new_Handle returned a handle with fake out of memory");

        const char* error_msg = kqt_Handle_get_error(0);
        fail_if(strlen(error_msg) == 0,
                "Memory allocation failure did not give an error message");
        fail_if(strstr(error_msg, "\"MemoryError\"") == NULL,
                "Error message on memory allocation failure was not a MemoryError");
    }

    // Make sure that we succeed with the error step far enough
    kqt_fake_out_of_memory(alloc_count);

    handle = kqt_new_Handle();
    fail_if(handle == 0,
            "kqt_new_Handle did not succeed with %ld allocations",
            alloc_count);
    kqt_del_Handle(handle);
    handle = 0;
}
END_TEST
#endif // KQT_LONG_TESTS


START_TEST(Aligned_alloc_returns_proper_base_address)
{
    uint8_t alignment = (uint8_t)_i;

    kqt_fake_out_of_memory(-1);

#define ALLOC_TEST_COUNT 4
    void* blocks[ALLOC_TEST_COUNT] = { NULL };

    for (int i = 0; i < ALLOC_TEST_COUNT; ++i)
    {
#define BLOCK_SIZE 16384
        blocks[i] = memory_alloc_items_aligned(char, BLOCK_SIZE, alignment);
        fail_if(blocks[i] == NULL, "Could not allocate aligned memory block");

        const intptr_t addr = (intptr_t)blocks[i];
        const intptr_t rem = addr % alignment;
        fail_if(rem != 0, "Incorrect alignment: got address %p, expected alignment %d",
                blocks[i], (int)alignment);

        char* block = blocks[i];
        for (int k = 0; k < BLOCK_SIZE; ++k)
            block[k] = (char)(80 + (k % 16));
#undef BLOCK_SIZE
    }

    for (int i = 0; i < ALLOC_TEST_COUNT; ++i)
        memory_free_aligned(blocks[i]);

#undef ALLOC_TEST_COUNT
}
END_TEST


Suite* Memory_suite(void)
{
    Suite* s = suite_create("Memory");

    const int timeout = DEFAULT_TIMEOUT;

    TCase* tc_oom = tcase_create("out_of_memory");
    suite_add_tcase(s, tc_oom);
    tcase_set_timeout(tc_oom, timeout);
    //tcase_add_checked_fixture(tc_oom, setup_empty, handle_teardown);

    TCase* tc_aligned = tcase_create("aligned");
    suite_add_tcase(s, tc_aligned);
    tcase_set_timeout(tc_aligned, timeout);

#ifdef KQT_LONG_TESTS
    tcase_set_timeout(tc_oom, LONG_TIMEOUT);
    tcase_add_test(tc_oom, Out_of_memory_at_handle_creation_fails_cleanly);
#endif

    tcase_add_loop_test(tc_aligned, Aligned_alloc_returns_proper_base_address, 2, 64);

    return s;
}


int main(void)
{
    Suite* suite = Memory_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


