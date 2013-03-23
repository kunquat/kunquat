# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
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
from pulseaudio import Pulseaudio
from pushaudio import Pushaudio

class TestDrivers(unittest.TestCase):

    def setUp(self):
        pass

    def _prefeed(self, driver):
        class gen():
            def generate(self,foo):
                pass
        driver.set_audio_generator(gen())
        driver.put_audio((10000*[0.1],10000*[0.1]))
        driver.start()
        sleep(1)
        driver.put_audio((10000*[0.1],10000*[0.1]))
        driver.stop()

    def test_pushaudio_prefeed(self):
        driver = Pushaudio()
        self._prefeed(driver)

    def test_pulseaudio_prefeed(self):
        driver = Pulseaudio()
        self._prefeed(driver)

if __name__ == '__main__':
    unittest.main()

