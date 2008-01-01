

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>

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
	fail_unless(rp->part == 0,
			"Reltime_init() set part to %ld instead of 0.", (long)rp->part);
}
END_TEST


#ifndef NDEBUG
START_TEST (init_break)
{
	Reltime_init(NULL);
}
END_TEST
#endif


Suite* Reltime_suite(void)
{
	Suite* s = suite_create("Reltime");
	TCase* tc_init = tcase_create("init");
	suite_add_tcase(s, tc_init);

	tcase_add_test(tc_init, init);

#ifndef NDEBUG
	tcase_add_test_raise_signal(tc_init, init_break, SIGABRT);
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


