# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2016-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time
import unittest

from .pulseaudio_async import Async


class TestPulseaudioAsync(unittest.TestCase):

    def _pa_callback(self, nframes):
        self._calls += 1
        sound = (nframes * 2) * [0.2]
        return sound

    def test_basics(self):
        self._calls = 0
        self._pa = Async(
                'PulseAudio Asynchronous Python Wrapper Unit Test',
                'Basic Test',
                self._pa_callback)
        self._pa.init()
        self._pa.play()
        time.sleep(0.2)
        self._pa.stop()
        self._pa.deinit()
        self.assertTrue(self._calls > 1)


if __name__ == '__main__':
    unittest.main()


