# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import unittest
from time import sleep


class SilentAudioSource():

    def set_audio_output(self, audio_output):
        self._audio_output = audio_output

    def generate_audio(self, nframes):
        silence = [0.1] * (nframes * 2)
        self._audio_output.put_audio(silence)


class StressTestAbstractDriver():

    def setUp(self):
        self._self._DriverClass = None
        self._cls_args = tuple()

    def test_rapid_reboot(self):
        for _ in range(10):
            driver = self._DriverClass(*self._cls_args)
            audio_source = SilentAudioSource()
            audio_source.set_audio_output(driver)
            driver.set_audio_source(audio_source)
            driver.start()
            sleep(0.2)
            driver.close()


