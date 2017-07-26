# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2016
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

from .pulseaudio import Simple


@unittest.skip('Causes tests to hang in Travis')
class TestPulseaudio(unittest.TestCase):

    def test_basics(self):
        self._pa = Simple('PulseAudio Simple Python Wrapper Unit Test', 'Basic Test')
        mono_sound = 200 * [0.2]
        for _ in range(10):
            self._pa.write(mono_sound, mono_sound)


if __name__ == '__main__':
    unittest.main()


