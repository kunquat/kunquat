# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import islice, izip_longest
import unittest

import kunquat.tracker.ui.model.tstamp as tstamp
import utils


class TestUtils(unittest.TestCase):

    def test_px_from_tstamp_from_px_is_identity(self):
        for px_per_beat in range(1, 600):
            for px_offset in range(2 * px_per_beat):
                def_ts = utils.get_tstamp_from_px(px_offset, px_per_beat)
                new_px_offset = utils.get_px_from_tstamp(def_ts, px_per_beat)
                self.assertEqual(px_offset, new_px_offset)

    def test_beat_is_divided_optimally(self):
        for px_per_beat in range(1, 300):
            offsets = [utils.get_tstamp_from_px(offset, px_per_beat)
                    for offset in range(px_per_beat)]
            self.assertEqual(offsets[0], 0)
            expected_px = 0
            lens = set()
            for start, stop in izip_longest(offsets, islice(offsets, 1, None)):
                if stop == None:
                    stop = tstamp.Tstamp(1, 0)
                end = stop - tstamp.Tstamp(0, 1)
                start_px = utils.get_px_from_tstamp(start, px_per_beat)
                end_px = utils.get_px_from_tstamp(end, px_per_beat)

                self.assertEqual(expected_px, start_px,
                        msg='start_px {} did not match expected_px {},'
                        ' px_per_beat: {}'.format(start_px, expected_px, px_per_beat))
                self.assertEqual(start_px, end_px,
                        msg='{} and {} resulted in different pixel offsets {} and {},'
                        ' px_per_beat: {}'.format(
                            start, end, start_px, end_px, px_per_beat))
                interval_len = end - start
                self.assertTrue(interval_len in lens or len(lens) < 2,
                        msg='Found third interval length {} at [{}, {}],'
                        ' existing: {}, px_per_beat: {}'.format(
                            interval_len, start, end, lens, px_per_beat))

                lens.add(interval_len)
                expected_px += 1

            lens_list = list(lens)
            if len(lens_list) == 2:
                self.assertTrue(abs(lens_list[0] - lens_list[1]) == tstamp.Tstamp(0, 1),
                        msg='Incorrect lengths: {}, px_per_beat: {}'.format(
                            lens_list, px_per_beat))


