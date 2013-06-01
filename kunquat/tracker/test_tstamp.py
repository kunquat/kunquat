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


class TestTstamp(unittest.TestCase):

    def _check_types(self, ts):
        self.assertIsInstance(ts, tstamp.Tstamp)
        self.assertIsInstance(ts, numbers.Real)
        self.assertIsInstance(ts.beats, (int, long))
        self.assertIsInstance(ts.rem, int)
        self.assertTrue(ts.rem >= 0)
        self.assertTrue(ts.rem < tstamp.BEAT)

    def _check_values(self, ts, beats, rem):
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
        ts = tstamp.Tstamp(0, tstamp.BEAT)
        self._check_types_and_values(ts, 1, 0)

        ts = tstamp.Tstamp(0, tstamp.BEAT + 1)
        self._check_types_and_values(ts, 1, 1)

        ts = tstamp.Tstamp(0, tstamp.BEAT * 2 - 1)
        self._check_types_and_values(ts, 1, tstamp.BEAT - 1)

        ts = tstamp.Tstamp(0, tstamp.BEAT * 2)
        self._check_types_and_values(ts, 2, 0)

        ts = tstamp.Tstamp(0, tstamp.BEAT * 2 + 1)
        self._check_types_and_values(ts, 2, 1)

        ts = tstamp.Tstamp(0, -1)
        self._check_types_and_values(ts, -1, tstamp.BEAT - 1)

        ts = tstamp.Tstamp(0, -tstamp.BEAT)
        self._check_types_and_values(ts, -1, 0)

        ts = tstamp.Tstamp(0, -tstamp.BEAT - 1)
        self._check_types_and_values(ts, -2, tstamp.BEAT - 1)

        ts = tstamp.Tstamp(0, -tstamp.BEAT * 2 + 1)
        self._check_types_and_values(ts, -2, 1)

        ts = tstamp.Tstamp(0, -tstamp.BEAT * 2)
        self._check_types_and_values(ts, -2, 0)

        ts = tstamp.Tstamp(0, -tstamp.BEAT * 2 - 1)
        self._check_types_and_values(ts, -3, tstamp.BEAT - 1)

    def test_init_from_tuple(self):
        for beats, rem in self._simple_init_values():
            ts = tstamp.Tstamp((beats, rem))
            self._check_types_and_values(ts, beats, rem)

    def test_init_from_list(self):
        for beats, rem in self._simple_init_values():
            ts = tstamp.Tstamp([beats, rem])
            self._check_types_and_values(ts, beats, rem)


if __name__ == '__main__':
    unittest.main()


