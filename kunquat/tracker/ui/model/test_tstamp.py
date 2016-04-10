# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import product
import numbers
import unittest

import tstamp


def default_order(x, y):
    return (x, y)


def flip(x, y):
    return (y, x)


class TestTstamp(unittest.TestCase):

    def _check_types(self, ts):
        self.assertIsInstance(ts, tstamp.Tstamp)
        self.assertIsInstance(ts, numbers.Real)
        self.assertIsInstance(ts.beats, int)
        self.assertIsInstance(ts.rem, int)
        self.assertTrue(ts.rem >= 0)
        self.assertTrue(ts.rem < tstamp.BEAT)

    def _check_values(self, ts, beats, rem):
        assert rem >= 0
        assert rem < tstamp.BEAT
        self.assertEqual(ts.beats, beats)
        self.assertEqual(ts.rem, rem)

    def _check_types_and_values(self, ts, beats, rem):
        self._check_types(ts)
        self._check_values(ts, beats, rem)

    def _simple_init_values(self):
        return ((b, r) for b in range(-4, 4) for r in range(4))

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
        for beats in range(-4, 4):
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

    def _test_oadd_int(self, order=default_order):
        arg1, arg2 = order(tstamp.Tstamp(), 0)
        self._check_types_and_values(arg1 + arg2, 0, 0)

        arg1, arg2 = order(tstamp.Tstamp(0, tstamp.BEAT // 4), 3)
        self._check_types_and_values(arg1 + arg2, 3, tstamp.BEAT // 4)

        arg1, arg2 = order(tstamp.Tstamp(0, tstamp.BEAT // 4), -3)
        self._check_types_and_values(arg1 + arg2, -3, tstamp.BEAT // 4)

    def test_add_int(self):
        self._test_oadd_int()

    def test_radd_int(self):
        self._test_oadd_int(flip)

    def _test_oadd_float(self, order=default_order):
        arg1, arg2 = order(tstamp.Tstamp(), 0.0)
        self._check_types_and_values(arg1 + arg2, 0, 0)

        arg1, arg2 = order(tstamp.Tstamp(0, tstamp.BEAT // 4), 0.5)
        self._check_types_and_values(arg1 + arg2, 0, 3 * tstamp.BEAT // 4)

        arg1, arg2 = order(tstamp.Tstamp(0, tstamp.BEAT // 4), -0.5)
        self._check_types_and_values(arg1 + arg2, -1, 3 * tstamp.BEAT // 4)

    def test_add_float(self):
        self._test_oadd_float()

    def test_radd_float(self):
        self._test_oadd_float(flip)

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

    def _test_omul_int(self, order=default_order):
        arg1, arg2 = order(tstamp.Tstamp(), 0)
        self._check_types_and_values(arg1 * arg2, 0, 0)

        arg1, arg2 = order(tstamp.Tstamp(3, 5), 1)
        self._check_types_and_values(arg1 * arg2, 3, 5)

        arg1, arg2 = order(tstamp.Tstamp(5, 7), 3)
        self._check_types_and_values(arg1 * arg2, 15, 21)

        arg1, arg2 = order(tstamp.Tstamp(2, 3), -2)
        self._check_types_and_values(arg1 * arg2, -5, tstamp.BEAT - 6)

    def test_mul_int(self):
        self._test_omul_int()

    def test_rmul_int(self):
        self._test_omul_int(flip)

    def _test_omul_float(self, order=default_order):
        arg1, arg2 = order(tstamp.Tstamp(), 0.0)
        self._check_types_and_values(arg1 * arg2, 0, 0)

        arg1, arg2 = order(tstamp.Tstamp(2), 2.25)
        self._check_types_and_values(arg1 * arg2, 4, tstamp.BEAT // 2)

    def test_mul_float(self):
        self._test_omul_float()

    def test_rmul_float(self):
        self._test_omul_float(flip)

    def test_mul_tstamp(self):
        ts = tstamp.Tstamp(3) * tstamp.Tstamp(0.5)
        self._check_types_and_values(ts, 1, tstamp.BEAT // 2)

        ts = tstamp.Tstamp(2) * tstamp.Tstamp(1.25)
        self._check_types_and_values(ts, 2, tstamp.BEAT // 2)

    def _tstamp_seeds_with_order(self):
        seeds = (-2, -1, 0, 1, 2)
        return product(enumerate(seeds), enumerate(seeds))

    def test_eq_tstamp(self):
        for ((i, a), (k, b)) in self._tstamp_seeds_with_order():
            ta = tstamp.Tstamp(float(a) * 0.5)
            tb = tstamp.Tstamp(float(b) * 0.5)
            if i == k:
                self.assertEqual(ta, tb)
            else:
                self.assertNotEqual(ta, tb)

    def _test_oeq_int(self, order=default_order):
        equals = [
                order(tstamp.Tstamp(), 0),
                order(tstamp.Tstamp(-1), -1),
                order(tstamp.Tstamp(1), 1),
                ]
        for (arg1, arg2) in equals:
            self.assertEqual(arg1, arg2)

        unequals = [
                order(tstamp.Tstamp(), -1),
                order(tstamp.Tstamp(), 1),
                order(tstamp.Tstamp(0.5), 0),
                order(tstamp.Tstamp(0.5), 1),
                ]
        for (arg1, arg2) in unequals:
            self.assertNotEqual(arg1, arg2)

    def test_eq_int(self):
        self._test_oeq_int()

    def test_req_int(self):
        self._test_oeq_int(flip)

    def _test_oeq_float(self, order=default_order):
        for ((i, a), (k, b)) in self._tstamp_seeds_with_order():
            ta = tstamp.Tstamp(float(a) * 0.5)
            arg1, arg2 = order(ta, float(b) * 0.5)
            if i == k:
                self.assertEqual(arg1, arg2)
            else:
                self.assertNotEqual(arg1, arg2)

    def test_eq_float(self):
        self._test_oeq_float()

    def test_req_float(self):
        self._test_oeq_float(flip)

    def test_eq_None(self):
        for source in (-1, -0.5, 0, 0.5, 1):
            self.assertNotEqual(tstamp.Tstamp(source), None)
            self.assertNotEqual(None, tstamp.Tstamp(source))

    def test_lt_tstamp(self):
        for ((i, a), (k, b)) in self._tstamp_seeds_with_order():
            arg1 = tstamp.Tstamp(float(a) * 0.5)
            arg2 = tstamp.Tstamp(float(b) * 0.5)
            if i < k:
                self.assertTrue(arg1 < arg2)
                self.assertTrue(arg2 > arg1)
            else:
                self.assertFalse(arg1 < arg2)
                self.assertFalse(arg2 > arg1)

    def _test_olt_int(self, order=default_order):
        for ((i, a), (k, b)) in self._tstamp_seeds_with_order():
            i, k = order(i, k)
            ta = tstamp.Tstamp(a)
            arg1, arg2 = order(ta, b)
            if i < k:
                self.assertTrue(arg1 < arg2)
                self.assertTrue(arg2 > arg1)
            else:
                self.assertFalse(arg1 < arg2)
                self.assertFalse(arg2 > arg1)

        if order == flip:
            self.assertTrue(0 < tstamp.Tstamp(0.5))
            self.assertFalse(0 > tstamp.Tstamp(0.5))
            self.assertFalse(1 < tstamp.Tstamp(0.5))
            self.assertTrue(1 > tstamp.Tstamp(0.5))
        else:
            self.assertFalse(tstamp.Tstamp(0.5) < 0)
            self.assertTrue(tstamp.Tstamp(0.5) > 0)
            self.assertTrue(tstamp.Tstamp(0.5) < 1)
            self.assertFalse(tstamp.Tstamp(0.5) > 1)

    def test_lt_int(self):
        self._test_olt_int()

    def test_rlt_int(self):
        self._test_olt_int(flip)

    def _test_olt_float(self, order=default_order):
        for ((i, a), (k, b)) in self._tstamp_seeds_with_order():
            i, k = order(i, k)
            ta = tstamp.Tstamp(float(a) * 0.5)
            arg1, arg2 = order(ta, float(b) * 0.5)
            if i < k:
                self.assertTrue(arg1 < arg2)
                self.assertTrue(arg2 > arg1)
            else:
                self.assertFalse(arg1 < arg2)
                self.assertFalse(arg2 > arg1)

    def test_lt_float(self):
        self._test_olt_float()

    def test_rlt_float(self):
        self._test_olt_float(flip)

    def test_le_tstamp(self):
        for ((i, a), (k, b)) in self._tstamp_seeds_with_order():
            arg1 = tstamp.Tstamp(float(a) * 0.5)
            arg2 = tstamp.Tstamp(float(b) * 0.5)
            if i <= k:
                self.assertTrue(arg1 <= arg2)
                self.assertTrue(arg2 >= arg1)
            else:
                self.assertFalse(arg1 <= arg2)
                self.assertFalse(arg2 >= arg1)

    def _test_ole_int(self, order=default_order):
        for ((i, a), (k, b)) in self._tstamp_seeds_with_order():
            i, k = order(i, k)
            ta = tstamp.Tstamp(a)
            arg1, arg2 = order(ta, b)
            if i <= k:
                self.assertTrue(arg1 <= arg2)
                self.assertTrue(arg2 >= arg1)
            else:
                self.assertFalse(arg1 <= arg2)
                self.assertFalse(arg2 >= arg1)

    def test_le_int(self):
        self._test_ole_int()

    def test_rle_int(self):
        self._test_ole_int(flip)

    def _test_ole_float(self, order=default_order):
        for ((i, a), (k, b)) in self._tstamp_seeds_with_order():
            i, k = order(i, k)
            ta = tstamp.Tstamp(float(a) * 0.5)
            arg1, arg2 = order(ta, float(b) * 0.5)
            if i <= k:
                self.assertTrue(arg1 <= arg2)
                self.assertTrue(arg2 >= arg1)
            else:
                self.assertFalse(arg1 <= arg2)
                self.assertFalse(arg2 >= arg1)

    def test_le_float(self):
        self._test_ole_float()

    def test_rle_float(self):
        self._test_ole_float(flip)

    def test_abs(self):
        ts = abs(tstamp.Tstamp())
        self._check_types_and_values(ts, 0, 0)

        ts = abs(tstamp.Tstamp(2, 3))
        self._check_types_and_values(ts, 2, 3)

        ts = abs(tstamp.Tstamp(-5, -7))
        self._check_types_and_values(ts, 5, 7)

    def test_sub_tstamp(self):
        ts = tstamp.Tstamp() - tstamp.Tstamp()
        self._check_types_and_values(ts, 0, 0)

        ts = tstamp.Tstamp(1) - tstamp.Tstamp(0, tstamp.BEAT // 4)
        self._check_types_and_values(ts, 0, 3 * tstamp.BEAT // 4)

        ts = tstamp.Tstamp(2) - tstamp.Tstamp(3)
        self._check_types_and_values(ts, -1, 0)

        ts = tstamp.Tstamp(5, 2) - tstamp.Tstamp(3, 7)
        self._check_types_and_values(ts, 1, tstamp.BEAT - 5)

        ts = tstamp.Tstamp(1, 2) - tstamp.Tstamp(-3, -4)
        self._check_types_and_values(ts, 4, 6)

    def test_sub_int(self):
        ts = tstamp.Tstamp() - 0
        self._check_types_and_values(ts, 0, 0)

        ts = tstamp.Tstamp(3) - 2
        self._check_types_and_values(ts, 1, 0)

        ts = tstamp.Tstamp(0, tstamp.BEAT // 4) - 1
        self._check_types_and_values(ts, -1, tstamp.BEAT // 4)

    def test_sub_float(self):
        ts = tstamp.Tstamp() - 0.0
        self._check_types_and_values(ts, 0, 0)

        ts = tstamp.Tstamp(1.5) - 0.75
        self._check_types_and_values(ts, 0, 3 * tstamp.BEAT // 4)

    def test_float(self):
        for float_source in (-1.0, -0.5, 0.0, 0.5, 1.0):
            from_ts = float(tstamp.Tstamp(float_source))
            self.assertIsInstance(from_ts, float)
            self.assertEqual(from_ts, float_source)

    def test_int(self):
        for int_source in range(-4, 4):
            from_ts = int(tstamp.Tstamp(int_source))
            self.assertIsInstance(from_ts, int)
            self.assertEqual(from_ts, int_source)

        for source in (-0.75, -0.5, -0.25, 0.25, 0.5, 0.75):
            from_ts = int(tstamp.Tstamp(source))
            self.assertIsInstance(from_ts, int)
            self.assertEqual(from_ts, 0)

        self.assertEqual(int(tstamp.Tstamp(-1, 1)), 0)
        self.assertEqual(int(tstamp.Tstamp(1, -1)), 0)

    def test_hash_int(self):
        for int_source in range(-256, 256):
            ts = tstamp.Tstamp(int_source)
            self.assertEqual(hash(ts), hash(int_source))

    def test_hash_float(self):
        for float_source in (-0.75, -0.5, -0.25, 0.25, 0.5, 0.75):
            ts = tstamp.Tstamp(float_source)
            self.assertEqual(hash(ts), hash(float_source))

    def test_repr(self):
        for source in (-1.25, -0.5, 0.0, 0.25, 1.0):
            ts = tstamp.Tstamp(source)
            ts_repr = repr(ts)
            self.assertIsInstance(ts_repr, str)
            self.assertEqual(ts_repr, "Tstamp({}, {})".format(ts.beats, ts.rem))


if __name__ == '__main__':
    unittest.main()


