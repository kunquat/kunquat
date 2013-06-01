# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import numbers
import tstamp
import unittest


def default_order(x, y):
    return (x, y)


def flip(x, y):
    return (y, x)


class TestTstamp(unittest.TestCase):

    def _check_types(self, ts):
        self.assertIsInstance(ts, tstamp.Tstamp)
        self.assertIsInstance(ts, numbers.Real)
        self.assertIsInstance(ts.beats, (int, long))
        self.assertIsInstance(ts.rem, int)
        self.assertTrue(ts.rem >= 0)
        self.assertTrue(ts.rem < tstamp.BEAT)

    def _check_values(self, ts, beats, rem):
        assert rem >= 0
        assert rem < tstamp.BEAT
        self.assertEquals(ts.beats, beats)
        self.assertEquals(ts.rem, rem)

    def _check_types_and_values(self, ts, beats, rem):
        self._check_types(ts)
        self._check_values(ts, beats, rem)

    def _simple_init_values(self):
        return ((b, r) for b in xrange(-4, 4) for r in xrange(4))

    def test_is_immutable(self):
        ts = tstamp.Tstamp()
        self._check_types(ts)
        self._check_types_and_values(ts, 0, 0)

        def modify_beats(ts):
            ts.beats = 1
        def modify_rem(ts):
            ts.rem = 2
        def modify_tuple(ts, index):
            ts[index] = 3

        self.assertRaises(AttributeError, modify_beats, ts)
        self._check_types_and_values(ts, 0, 0)
        self.assertRaises(AttributeError, modify_rem, ts)
        self._check_types_and_values(ts, 0, 0)
        self.assertRaises(TypeError, modify_tuple, ts, 0)
        self._check_types_and_values(ts, 0, 0)
        self.assertRaises(TypeError, modify_tuple, ts, 1)
        self._check_types_and_values(ts, 0, 0)

    def test_init_without_args_creates_zero(self):
        ts = tstamp.Tstamp()
        self._check_types_and_values(ts, 0, 0)

    def test_init_with_single_int_arg_sets_beats(self):
        for beats in xrange(-4, 4):
            ts = tstamp.Tstamp(beats)
            self._check_types_and_values(ts, beats, 0)

    def test_init_with_two_int_args_sets_beats_and_rem(self):
        for beats, rem in self._simple_init_values():
            ts = tstamp.Tstamp(beats, rem)
            self._check_types_and_values(ts, beats, rem)

        ts = tstamp.Tstamp(0, tstamp.BEAT - 1)
        self._check_types_and_values(ts, 0, tstamp.BEAT - 1)

    def test_init_with_single_float_arg_sets_beats_and_rem(self):
        ts = tstamp.Tstamp(0.0)
        self._check_types_and_values(ts, 0, 0)

        ts = tstamp.Tstamp(-0.25)
        self._check_types_and_values(ts, -1, 3 * tstamp.BEAT // 4)

        ts = tstamp.Tstamp(0.25)
        self._check_types_and_values(ts, 0, tstamp.BEAT // 4)

    def test_init_with_overflowing_rem_is_resolved(self):
        ts = tstamp.Tstamp(1, tstamp.BEAT)
        self._check_types_and_values(ts, 2, 0)

        ts = tstamp.Tstamp(1, tstamp.BEAT + 1)
        self._check_types_and_values(ts, 2, 1)

        ts = tstamp.Tstamp(1, tstamp.BEAT * 2 - 1)
        self._check_types_and_values(ts, 2, tstamp.BEAT - 1)

        ts = tstamp.Tstamp(1, tstamp.BEAT * 2)
        self._check_types_and_values(ts, 3, 0)

        ts = tstamp.Tstamp(1, tstamp.BEAT * 2 + 1)
        self._check_types_and_values(ts, 3, 1)

        ts = tstamp.Tstamp(1, -1)
        self._check_types_and_values(ts, 0, tstamp.BEAT - 1)

        ts = tstamp.Tstamp(1, -tstamp.BEAT)
        self._check_types_and_values(ts, 0, 0)

        ts = tstamp.Tstamp(1, -tstamp.BEAT - 1)
        self._check_types_and_values(ts, -1, tstamp.BEAT - 1)

        ts = tstamp.Tstamp(1, -tstamp.BEAT * 2 + 1)
        self._check_types_and_values(ts, -1, 1)

        ts = tstamp.Tstamp(1, -tstamp.BEAT * 2)
        self._check_types_and_values(ts, -1, 0)

        ts = tstamp.Tstamp(1, -tstamp.BEAT * 2 - 1)
        self._check_types_and_values(ts, -2, tstamp.BEAT - 1)

    def test_init_from_tuple(self):
        for beats, rem in self._simple_init_values():
            ts = tstamp.Tstamp((beats, rem))
            self._check_types_and_values(ts, beats, rem)

    def test_init_from_list(self):
        for beats, rem in self._simple_init_values():
            ts = tstamp.Tstamp([beats, rem])
            self._check_types_and_values(ts, beats, rem)

    def test_init_from_tstamp(self):
        for beats, rem in self._simple_init_values():
            ts = tstamp.Tstamp(tstamp.Tstamp(beats, rem))
            self._check_types_and_values(ts, beats, rem)

    def test_add_tstamp(self):
        ts = tstamp.Tstamp() + tstamp.Tstamp()
        self._check_types_and_values(ts, 0, 0)

        ts = tstamp.Tstamp(2) + tstamp.Tstamp(3)
        self._check_types_and_values(ts, 5, 0)

        ts = tstamp.Tstamp(2) + tstamp.Tstamp(-3)
        self._check_types_and_values(ts, -1, 0)

        ts = tstamp.Tstamp(0, 2) + tstamp.Tstamp(0, 3)
        self._check_types_and_values(ts, 0, 5)

        ts = tstamp.Tstamp(0, 2) + tstamp.Tstamp(0, -3)
        self._check_types_and_values(ts, -1, tstamp.BEAT - 1)

        ts = tstamp.Tstamp(0, tstamp.BEAT // 2) + tstamp.Tstamp(0, tstamp.BEAT // 2)
        self._check_types_and_values(ts, 1, 0)

        ts = tstamp.Tstamp(2, 3) + tstamp.Tstamp(-2, -3)
        self._check_types_and_values(ts, 0, 0)

    def _test_nadd_int(self, order=default_order):
        arg1, arg2 = order(tstamp.Tstamp(), 0)
        self._check_types_and_values(arg1 + arg2, 0, 0)

        arg1, arg2 = order(tstamp.Tstamp(0, tstamp.BEAT // 4), 3)
        self._check_types_and_values(arg1 + arg2, 3, tstamp.BEAT // 4)

        arg1, arg2 = order(tstamp.Tstamp(0, tstamp.BEAT // 4), -3)
        self._check_types_and_values(arg1 + arg2, -3, tstamp.BEAT // 4)

    def test_add_int(self):
        self._test_nadd_int()

    def test_radd_int(self):
        self._test_nadd_int(flip)

    def _test_nadd_float(self, order=default_order):
        arg1, arg2 = order(tstamp.Tstamp(), 0.0)
        self._check_types_and_values(arg1 + arg2, 0, 0)

        arg1, arg2 = order(tstamp.Tstamp(0, tstamp.BEAT // 4), 0.5)
        self._check_types_and_values(arg1 + arg2, 0, 3 * tstamp.BEAT // 4)

        arg1, arg2 = order(tstamp.Tstamp(0, tstamp.BEAT // 4), -0.5)
        self._check_types_and_values(arg1 + arg2, -1, 3 * tstamp.BEAT // 4)

    def test_add_float(self):
        self._test_nadd_float()

    def test_radd_float(self):
        self._test_nadd_float(flip)

    def test_pos(self):
        ts = +tstamp.Tstamp()
        self._check_types_and_values(ts, 0, 0)

        ts = +tstamp.Tstamp(1, 2)
        self._check_types_and_values(ts, 1, 2)

        ts = +tstamp.Tstamp(-5, 7)
        self._check_types_and_values(ts, -5, 7)

    def test_neg(self):
        ts = -tstamp.Tstamp()
        self._check_types_and_values(ts, 0, 0)

        ts = -tstamp.Tstamp(1, 0)
        self._check_types_and_values(ts, -1, 0)

        ts = -tstamp.Tstamp(2, 3)
        self._check_types_and_values(ts, -3, tstamp.BEAT - 3)

        ts = -tstamp.Tstamp(-5, 7)
        self._check_types_and_values(ts, 4, tstamp.BEAT - 7)

    def _test_nmul_int(self, order=default_order):
        arg1, arg2 = order(tstamp.Tstamp(), 0)
        self._check_types_and_values(arg1 * arg2, 0, 0)

        arg1, arg2 = order(tstamp.Tstamp(3, 5), 1)
        self._check_types_and_values(arg1 * arg2, 3, 5)

        arg1, arg2 = order(tstamp.Tstamp(5, 7), 3)
        self._check_types_and_values(arg1 * arg2, 15, 21)

        arg1, arg2 = order(tstamp.Tstamp(2, 3), -2)
        self._check_types_and_values(arg1 * arg2, -5, tstamp.BEAT - 6)

    def test_mul_int(self):
        self._test_nmul_int()

    def test_rmul_int(self):
        self._test_nmul_int(flip)

    def _test_nmul_float(self, order=default_order):
        arg1, arg2 = order(tstamp.Tstamp(), 0.0)
        self._check_types_and_values(arg1 * arg2, 0, 0)

        arg1, arg2 = order(tstamp.Tstamp(2), 2.25)
        self._check_types_and_values(arg1 * arg2, 4, tstamp.BEAT // 2)

    def test_mul_float(self):
        self._test_nmul_float()

    def test_rmul_float(self):
        self._test_nmul_float(flip)

    def test_mul_tstamp(self):
        ts = tstamp.Tstamp(3) * tstamp.Tstamp(0.5)
        self._check_types_and_values(ts, 1, tstamp.BEAT // 2)

        ts = tstamp.Tstamp(2) * tstamp.Tstamp(1.25)
        self._check_types_and_values(ts, 2, tstamp.BEAT // 2)


if __name__ == '__main__':
    unittest.main()


